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
  
  var layoutParentId = 'cwrc_wrapper'; // specified in islandora-cwrc-writer.tpl.php
  
  function cwrcWriterInit($, Writer, Layout, Dialogs) {
      'use strict';
      var writer, config;
      config = Drupal.settings.CWRCWriter;
      config.id = config.id || layoutParentId;
      
      config.layout = Layout;
      config.entityLookupDialogs = Dialogs;
      config.storageDialogs = {
          save: function(writer) {
              var docId = writer.currentDocId;
              var docText = writer.converter.getDocumentContent(true);
              $.ajax({
                  url : writer.baseUrl+'editor/documents/'+docId,
                  type: 'PUT',
                  dataType: 'json',
                  data: {'doc':docText, 'schema':writer.schemaManager.schemas[writer.schemaManager.schemaId]['pid']},
                  success: function(data, status, xhr) {
                      writer.dialogManager.show('message', {
                          title : 'Document Saved',
                          msg : docId + ' was saved successfully.'
                      });
                      writer.event('documentSaved').publish();
                      window.location.hash = '#'+docId;
                      writer.editor.isNotDirty = true;
                  },
                  error: function(xhr, status, error) {
                      displayError(xhr, docId);
                  }
              });
          },
          load: function(writer) {
              var docId;
              if (config.documents.length) {
                  // Overlay can completely mangle the hash, so we can't rely on it.
                  docId = writer.currentDocId ? writer.currentDocId : config.documents[0];
              }
              if (docId != null) {
                  writer.currentDocId = docId;
                  writer.event('loadingDocument').publish();
                  
                  $.ajax({
                      url: writer.baseUrl+'editor/documents/'+docId,
                      type: 'GET',
                      success: function(doc, status, xhr) {
                          window.location.hash = '#'+docId;
                          writer.converter.processDocument(doc, config.schemaId);
                      },
                      error: function(xhr, status, error) {
                          writer.dialogManager.show('message', {
                              title: 'Error',
                              msg: 'An error ('+status+') occurred and '+docId+' was not loaded.',
                              type: 'error'
                          });
                          writer.currentDocId = null;
                          
                          writer.event('documentLoaded').publish(false, null);
                      },
                      dataType: 'xml'
                  });
              } else {
                  writer.dialogManager.show('message', {
                      title: 'Error',
                      msg: 'No document to load',
                      type: 'error'
                  });
              }
          }
      }
      
      writer = new Writer(config);
      writer.init(config.id);
      
        /**
         * Re-write the Delegator save and exit to do things.
         *
         * @see Delegator.saveAndExit
         */
        var saveAndExit = function(callback) {
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
        var displayError = function(xhr, docId) {
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

      // Replace the baseUrl after object construction since it's hard-coded.
      writer.baseUrl = config.baseUrl;
      // Hold onto a reference for safe keeping.
      Drupal.CWRCWriter.writer = writer;
      writer.event('writerInitialized').subscribe(function (writer) {

          if (typeof config.initial_mode !== 'undefined') {
              if (config.initial_mode == 'annotate') {
                writer.isAnnotator = true;
                writer.layout.ui.open('west');
                writer.showToolbar();
                writer.editor.plugins.cwrc_contextmenu.disabled = false;
                writer.editor.plugins.cwrc_contextmenu.entityTagsOnly = true;
              }
              else if (config.initial_mode == 'read') {
                writer.isAnnotator = false;
                writer.layout.ui.open('west');
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
            
            writer.storageDialogs.load(writer);
      });
  } // end cwrcWriterInit
  
  
  
  
  
  // Triggers the loading of CWRC-Writer.
  Drupal.behaviors.cwrcWriterLoad = {
    attach: function (context, settings) {
      $("#"+layoutParentId, context).once('cwrcWriterLoad', function () {
        // We have to set the height explicitly since the CWRC-Writer assumes it
        // has the full body.
        if(!window.frameElement) {
          var height = $(document.body).height()-40;
          $('#'+layoutParentId).height(height);
        }
        
        cwrcWriterInit($, Drupal.CWRCWriter, Drupal.CWRCWriterLayout, Drupal.CWRCWriterDialogs);
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
