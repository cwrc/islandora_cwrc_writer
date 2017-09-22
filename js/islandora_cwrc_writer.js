/*jshint browser: true*/
/*global jQuery, Drupal, setupLayoutAndModules, CwrcApi:true*/
/**
 * @file
 * Loads the CWRC-Writer.
 */
/**
 * CWRC-Writer global callback used to configure the CWRC-Writer.
 *
 * @param Writer
 * @param Delegator
 */
Drupal.CWRCWriter = Drupal.CWRCWriter || {};
(function ($) {
  'use strict';

  // layout class
  function Layout(w) {
      this.w = w;
      this.ui = null;
      this.mode = null; // 'reader' or 'annotator'
  }
  
  Layout.prototype = {
      constructor: Layout,
      
      /**
       * Initialize the layout
       * @param {jQuery|Element} container The container that will contain the layout
       * @param {String} textareaId The ID to assign to the editor textarea
       * @returns jquery-ui class
       */
      init: function(container, textareaId) {
          var cwrcName = 'CWRC-Writer';
          var version = '0.9';
          var southTabs = ''+
          '<div class="cwrc ui-layout-south">'+
              '<div id="southTabs" class="tabs">'+
                  '<ul>'+
                      '<li><a href="#validation">Validation</a></li>'+
                      '<li><a href="#selection">Markup</a></li>'+
                  '</ul>'+
                  '<div id="southTabsContent" class="ui-layout-content"></div>'+
              '</div>'+
          '</div>';
          if (this.w.isReadOnly) {
              this.mode = 'reader';
              cwrcName = 'CWRC-Reader';
              southTabs = '';
          }
          $(container).html(
              '<div id="cwrc_loadingMask" class="cwrc"><div>Loading '+cwrcName+'</div></div>'+
              '<div id="cwrc_wrapper">'+
                  '<div id="cwrc_header" class="cwrc ui-layout-north">'+
                      '<div id="headerParent" class="ui-widget">'+
                          '<a id="titleLink" href="http://www.cwrc.ca" target="_blank">'+cwrcName+' v.'+version+'</a>'+
                          '<div id="headerButtons"></div>'+
                      '</div>'+
                  '</div>'+
                  '<div class="cwrc ui-layout-west">'+
                      '<div id="westTabs" class="tabs">'+
                          '<ul>'+
                              '<li><a href="#entities">Entities</a></li>'+
                              '<li><a href="#structure">Structure</a></li>'+
                              '<li><a href="#relations">Relations</a></li>'+
                          '</ul>'+
                          '<div id="westTabsContent" class="ui-layout-content"></div>'+
                      '</div>'+
                  '</div>'+
                  '<div id="cwrc_main" class="ui-layout-center">'+
                      '<div class="ui-layout-center">'+
                          '<form method="post" action="">'+
                              '<textarea id="'+textareaId+'" name="editor" class="tinymce"></textarea>'+
                          '</form>'+
                      '</div>'+
                      southTabs+
                  '</div>'+
              '</div>'
          );
          
          this.ui = $('#cwrc_wrapper').layout({
              defaults: {
                  maskIframesOnResize: true,
                  resizable: true,
                  slidable: false,
                  fxName: 'none' // 'slide'
              },
              north: {
                  size: 35,
                  spacing_open: 0,
                  minSize: 35,
                  maxSize: 60,
                  closable: false
              },
              west: {
                  size: 'auto',
                  minSize: 325,
                  onresize: function(region, pane, state, options) {
                      var tabsHeight = $('#westTabs > ul').outerHeight();
                      $('#westTabsContent').height(state.layoutHeight - tabsHeight);
  //                    $.layout.callbacks.resizeTabLayout(region, pane);
                  }
              }
          });
          
          this.ui.panes.center.layout({
              defaults: {
                  maskIframesOnResize: true,
                  resizable: true,
                  slidable: false
              },
              center: {
                  onresize: function(region, pane, state, options) {
                      var container = $(this.w.editor.getContainer());
                      var toolbar = container.find('.mce-toolbar-grp').first();
                      var iframe = container.find('iframe').first();
                      
                      var uiHeight = 2+(container.outerHeight()-container.height());
                      
                      if (toolbar.is(':visible')) uiHeight += toolbar.outerHeight();
                      
                      var uiWidth = container.outerWidth()-container.width();
                      
  //                    container.height(state.layoutHeight-uiHeight).width(state.layoutWidth-uiWidth);
                      iframe.height(state.layoutHeight-uiHeight).width(state.layoutWidth-uiWidth);
                  }.bind(this)
              },
              south: {
                  size: 250,
                  resizable: true,
                  initClosed: true,
                  activate: function(event, ui) {
                      $.layout.callbacks.resizeTabLayout(event, ui);
                  },
                  onresize: function(region, pane, state, options) {
                      var tabsHeight = $('#southTabs > ul').outerHeight();
                      $('#southTabsContent').height(state.layoutHeight - tabsHeight);
                  }
              }
          });
          
          this.w.layoutModules.addStructureTreePanel(this.w, 'westTabsContent');
          this.w.layoutModules.addEntitiesListPanel(this.w, 'westTabsContent');
          this.w.layoutModules.addRelationsListPanel(this.w, 'westTabsContent');
          
          if (!this.w.isReadOnly) {
              this.w.layoutModules.addValidationPanel(this.w, 'southTabsContent');
              this.w.layoutModules.addSelectionPanel(this.w, 'southTabsContent');
  
          }
          
          $('#westTabs').tabs({
              active: 1,
              activate: function(event, ui) {
                  $.layout.callbacks.resizeTabLayout(event, ui);
              },
              create: function(event, ui) {
                  $('#westTabs').parent().find('.ui-corner-all:not(button)').removeClass('ui-corner-all');
              }
          });
          if (!this.w.isReadOnly) {
              $('#southTabs').tabs({
                  active: 1,
                  activate: function(event, ui) {
                      $.layout.callbacks.resizeTabLayout(event, ui);
                  },
                  create: function(event, ui) {
                      $('#southTabs').parent().find('.ui-corner-all:not(button)').removeClass('ui-corner-all');
                  }
              });
          }
          
          var isLoading = false;
          var doneLayout = false;
          
          var onLoad = function() {
              isLoading = true;
              this.w.event('loadingDocument').unsubscribe(onLoad);
          }.bind(this);
          var onLoadDone = function() {
              isLoading = false;
              if (doneLayout) {
                  $('#cwrc_loadingMask').fadeOut();
                  this.w.event('documentLoaded').unsubscribe(onLoadDone);
                  doResize();
              }
          }.bind(this);
          var doResize = function() {
              this.ui.options.onresizeall_end = function() {
                  doneLayout = true;
                  if (isLoading === false) {
                      $('#cwrc_loadingMask').fadeOut();
                      this.ui.options.onresizeall_end = null;
                  }
                  if (this.w.isReadOnly) {
                      if ($('#annotateLink').length === 0) {
                          $('#headerLink').hide();
                          $('#headerButtons').append('<div id="annotateLink"><h2>Annotate</h2></div>');
                          
                          $('#annotateLink').click(function(e) {
                              if (this.mode === 'reader') {
                                  // TODO check credentials
                                  this.activateAnnotator();
                                  $('h2', e.currentTarget).text('Read');
                              } else {
                                  this.activateReader();
                                  $('h2', e.currentTarget).text('Annotate');
                              }
                          }.bind(this));
                          
                          this.w.settings.hideAdvanced();
                          
                          this.activateReader();
                      }
                  }
              }.bind(this);
              this.ui.resizeAll(); // now that the editor is loaded, set proper sizing
          }.bind(this);
          
          this.w.event('loadingDocument').subscribe(onLoad);
          this.w.event('documentLoaded').subscribe(onLoadDone);
          this.w.event('writerInitialized').subscribe(doResize);
          
          return this.ui;
      },
      
      activateReader: function() {
          this.w.isAnnotator = false;
          this.ui.open('west');
          this.w.hideToolbar();
          
          this.w.editor.plugins.cwrc_contextmenu.disabled = true;
          
          this.mode = 'reader';
      },
      
      activateAnnotator: function() {
          this.w.isAnnotator = true;
          this.ui.open('west');
          this.w.showToolbar();
          
          this.w.editor.plugins.cwrc_contextmenu.disabled = false;
          this.w.editor.plugins.cwrc_contextmenu.entityTagsOnly = true;
          
          this.mode = 'annotator';
      }
  }
  
  
  function cwrcWriterInit($, Writer) {
      'use strict';
      var writer, config;
      config = Drupal.settings.CWRCWriter;
      config.id = config.id || 'editor';
      
      config.layout = Layout;
      config.entityLookupDialogs;
      
      writer = new Writer(config);
      writer.init(config.id);
      
      writer.delegator = {};

      /**
       * Re-write the Delegator save to have schema info.
       *
       * @see Delegator.saveDocument
       */
      writer.delegator.saveDocument = function(docId, callback) {
        var docText = writer.converter.getDocumentContent(true);
        $.ajax({
          url : writer.baseUrl+'editor/documents/'+docId,
          type: 'PUT',
          dataType: 'json',
          data: {'doc':docText, 'schema':writer.schemaManager.schemas[writer.schemaManager.schemaId]['pid']},
          success: function(data, status, xhr) {
            writer.dialogManager.show('message', {
              title: 'Document Saved',
              msg: docId+' was saved successfully.'
            });
            window.location.hash = '#'+docId;
            if (callback) {
              callback.call(writer, true);
            }
            writer.event('documentSaved').publish();
            // Force the state to be clean, which has to be after the
            // window.location.hash is updated otherwise it may reset to the dirty
            // state.
            writer.editor.isNotDirty = true;
          },
          error: function(xhr, status, error) {
            writer.delegator.displayError(xhr, docId);
            if (callback) {
              callback.call(writer, false);
            }
          }
        });
      };

        /**
         * Re-write the Delegator save and exit to do things.
         *
         * @see Delegator.saveAndExit
         */
        writer.delegator.saveAndExit = function(callback) {
            var docText = writer.converter.getDocumentContent(true);
            $.ajax({
              url : writer.baseUrl+'editor/documents/'+writer.currentDocId,
              type: 'PUT',
              dataType: 'json',
              data: {'doc':docText, 'schema':writer.schemaManager.schemas[writer.schemaManager.schemaId]['pid']},
              success: function(data, status, xhr) {
                  // XXX: Force the state to be clean directly after the "save"
                  // occurs.
                  writer.editor.isNotDirty = true;

                  $.ajax({
                      url: Drupal.settings.basePath+'islandora/rest/v1/object/'+writer.currentDocId+'/lock',
                      type: 'DELETE',
                      success: function(data, status, xhr) {
                          window.location = Drupal.settings.basePath+'islandora/object/'+writer.currentDocId
                      },
                      error: function() {
                          writer.delegator.displayError(xhr, writer.currentDocId);
                      }
                  })
              },
              error: function(xhr, status, error) {
                  writer.delegator.displayError(xhr, writer.currentDocId);
                  if (callback) {
                      callback.call(writer, false);
                  }
                }
           });
        };

        /**
         * Utility function to display errors that occur during REST requests.
         */
        writer.delegator.displayError = function(xhr, docId) {
            var params = {
                '@docid': docId
            }

            var msg = Drupal.t('An error occurred and @docid was not saved.', params);
            if (typeof xhr.responseText != 'undefined') {
                var responseText = jQuery.parseJSON(xhr.responseText);
                var responseparams = {
                    '!responseText': responseText.message
                }
                var msg = msg.concat(' ' + Drupal.t('Additional info: !responseText', responseparams))
            }
            writer.dialogManager.show('message', {
                title: 'Error',
                msg: msg,
                type: 'error'
            });
        };

        /**
         * Override this so we can pass a schemaId to loadDocument.
         */
        writer.fileManager.loadInitialDocument = function(start, schemaId) {
          start = start.substr(1);
          if (start === 'load') {
            writer.dialogManager.filemanager.showLoader();
          } else if (start.match(/^templates\//) !== null) {
            start += '.xml';
            writer.fileManager.loadTemplate(start);
          } else if (start !== '') {
            writer.fileManager.loadDocument(start, schemaId);
          } else if (writer.initialConfig.defaultDocument) {
            writer.fileManager.loadInitialDocument('#'+writer.initialConfig.defaultDocument);
          }
        };

        /**
         * Override this so we can pass a schemaId to processDocument.
         */
        writer.fileManager.loadDocument = function(docName, schemaId) {
          writer.currentDocId = docName;
          writer.event('loadingDocument').publish();
          writer.delegator.loadDocument(docName, function(xml) {
            if (xml != null) {
              writer.converter.processDocument(xml, schemaId);
            } else {
              writer.currentDocId = null;
            }
          });
        };

      // Replace the baseUrl after object construction since it's hard-coded.
      writer.baseUrl = config.baseUrl;
      // Hold onto a reference for safe keeping.
      Drupal.CWRCWriter.writer = writer;
      writer.event('writerInitialized').subscribe(function (writer) {

          if (typeof config.initial_mode !== 'undefined') {
              if (config.initial_mode == 'annotate') {
                writer.isAnnotator = true;
                writer.layout.open('west');
                writer.showToolbar();
                writer.editor.plugins.cwrc_contextmenu.disabled = false;
                writer.editor.plugins.cwrc_contextmenu.entityTagsOnly = true;
              }
              else if (config.initial_mode == 'read') {
                writer.isAnnotator = false;
                writer.layout.open('west');
                writer.hideToolbar();
                writer.editor.plugins.cwrc_contextmenu.disabled = true;
              }
            }
            // Replace the show loader with our own function which can handle how we
            // load documents, such that it will be drupal aware.
            // broken by https://github.com/cwrc/CWRC-Writer/commit/c6c660bdba21c071098e76e3992e8a50a6658d39
            //writer.dialogManager.filemanager.showLoader = Drupal.CWRCWriter.dialogManager.filemanager.showLoader($, writer);

            // Log all loaded documents.
            writer.event('documentLoaded').subscribe(function () {
              // Update the select field with the new value if possible.
              var $select = $('#islandora-cwrc-document-select'),
                $navigation = $('#islandora-cwrc-document-nav');
              $select.val(Drupal.CWRCWriter.writer.currentDocId);
              $('li a', $navigation).removeClass('disabled');
              if ($('option:selected', $select).is('option:last-child')) {
                $('li.next a', $navigation).addClass('disabled');
              }
              if ($('option:selected', $select).is('option:first-child')) {
                $('li.prev a', $navigation).addClass('disabled');
              }
              // Force resize, as it's needed when the layout is done in an iframe
              // as it expects.
              setTimeout(writer.layout.resizeAll, 500);
            });
            if (config.documents.length) {
              // Overlay can completely mangle the hash, so we can't rely on it.
              window.location.hash = writer.currentDocId ? writer.currentDocId : config.documents[0];
            }
            if (window.location.hash) {
              writer.fileManager.loadInitialDocument(window.location.hash, config.schemaId);
            }
      });
    }
  
  // Triggers the loading of CWRC-Writer.
  Drupal.behaviors.cwrcWriterLoad = {
    attach: function (context, settings) {
      $("#cwrc_wrapper", context).once('cwrcWriterLoad', function () {
        // We have to set the height explicitly since the CWRC-Writer assumes it
        // has the full body.
        if(!window.frameElement) {
          $('#cwrc_wrapper').height(1000);
        }
        
        var url = settings.CWRCWriter.cwrcRootUrl+'js/app.js';
        $.getScript(url, function() {
            cwrcWriterInit.call(window, $, Drupal.CWRCWriter);
        })
        
      });
   }
  };

  // Attach behavior to the select field in the header so the user can change
  // documents.
  Drupal.behaviors.cwrcWriterDocumentSelect = {
    attach: function (context, settings) {
      $('#islandora-cwrc-document-select', context).once('islandora-cwrc-writer-select-document', function () {
          var $select = $(this);
          if (!$select.hasClass('processed')) {
                  // Any time this element changes reload the document.
                  $select.change(function (e) {
                    if (Drupal.CWRCWriter.writer !== undefined) {
                      Drupal.CWRCWriter.writer.fileManager.loadDocument($('option:selected', this).val());
                    }
                  });
                  $select.addClass('processed');
          }
      });
    }
  };
  // This handle the navigation links, and makes use of the select field for
  // updating the document.
  Drupal.behaviors.cwrcWriterDocumentNavigation = {
    attach: function (context, settings) {
      // @todo use once here.
      var $navigation = $('#islandora-cwrc-document-nav'),
        $select = $('#islandora-cwrc-document-select');
      // Handle the navigation links.
      if (!$navigation.hasClass('processed')) {
        // Update the disable class based on the value of the select field.
        $select.change(function (e) {
          var $selected = $('option:selected', this);
          $('li a', $navigation).removeClass('disabled');
          if ($selected.is('option:last-child')) {
            $('li.next a', $navigation).addClass('disabled');
          }
          if ($selected.is('option:first-child')) {
            $('li.prev a', $navigation).addClass('disabled');
          }
        });
        $('li.prev a', $navigation).click(function (event) {
          $('option:selected', $select).prev().attr('selected', 'selected');
          $select.change();
          event.preventDefault();
        });
        $('li.next a', $navigation).click(function (event) {
          $('option:selected', $select).next().attr('selected', 'selected');
          $select.change();
          event.preventDefault();
        });
        $navigation.addClass('processed');
      }
    }
  };

}(jQuery));

/**
 * CWRC-Writer global callback used to configure the CWRC-Writer.
 *
 * @param {jQuery} $
 * @param Writer
 */
// @ignore style_camel_case:function

