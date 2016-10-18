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

  // Triggers the loading of CWRC-Writer.
  Drupal.behaviors.cwrcWriterLoad = {
    attach: function (context, settings) {
      $("#cwrc_wrapper", context).once('cwrcWriterLoad', function () {
        // We have to set the height explicitly since the CWRC-Writer assumes it
        // has the full body.
        if(!window.frameElement) {
          $('#cwrc_wrapper').height(1000);
        }
        // Set the baseUrl, which will be used to load all the required javascript documents.
        require.config({
          baseUrl: settings.CWRCWriter.cwrcRootUrl + 'js',
        });

        // Load required jQuery in noConflict mode.
        define('jquery-private', ['jquery'], function ($) {
          return $.noConflict(true);
        });

        // Get required jQuery and initialize the CWRC-Writer.
        require(['jquery', 'knockout'], function($, knockout) {
          window.ko = knockout; // requirejs shim isn't working for knockout

          require(['writer',
            'delegator',
            'jquery.layout',
            'jquery.tablayout'
          ], function(Writer, Delegator) {
            $(function() {
              cwrcWriterInit.call(window, $, Writer, Delegator);
            });
          });
        });
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
 * @param Delegator
 */
// @ignore style_camel_case:function
function cwrcWriterInit($, Writer, Delegator) {
  'use strict';
  var writer, config;
  config = Drupal.settings.CWRCWriter;
  config.id = config.id || 'editor';
  config.delegator = Delegator;
  writer = new Writer(config);
  writer.init(config.id);

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

    /*
     * code cause the RDF Overlap dialog to appear twice - following codeblock
     * removed as per (with details on how to implement properly 
     * https://github.com/cwrc/CWRC-Writer/issues/420 - 2016-10-01
     * code seems to account for a situation where the user would change 
     * the schema through the settings dialog, which would require clearing 
     * of the current document.
     *
     */
/*
    // When we change the schema we should update the document.
    writer.event('schemaLoaded').subscribe(function () {
      var defaultTEI =
          '<TEI xmlns="http://www.tei-c.org/ns/1.0">' +
          '<teiHeader>' +
          '<fileDesc>' +
          '<titleStmt>' +
          '<title></title>' +
          '</titleStmt>' +
          '<publicationStmt>' +
          '<p/>' +
          '</publicationStmt>' +
          '<sourceDesc>' +
          '<p></p>' +
          '</sourceDesc>' +
          '</fileDesc>' +
          '</teiHeader>' +
          '<text>' +
          '<body><p>Paste or type your text here</p></body>' +
          '</text>' +
          '</TEI>';
      var defaultXML;
      var root;
      var defaultDoc;
      var doc;
      var parser = new DOMParser();
      var content = writer.editor.getContent();
      // Only reload content if there is actually content.
      if (content === '') {
        return;
      }
      // We have to parse as HTML as HTML entities will break XML parsing in
      // firefox.
      doc = parser.parseFromString(content, 'text/html');
      // Embeds the document in the body tag.
      root = doc.body.firstElementChild.getAttribute('_tag');
      switch (writer.root) {
      case 'TEI':
        if (root !== 'TEI') {
          defaultDoc = parser.parseFromString(defaultTEI, 'text/xml');
          writer.converter.doProcessing(defaultDoc);
        }
        break;
      default:
        if (root !== writer.root) {
          defaultXML = '<'+ writer.root + '></'+ writer.root + '>';
          defaultDoc = parser.parseFromString(defaultXML, 'text/xml');
          writer.converter.doProcessing(defaultDoc);
        }
        break;
      }
    });

    */ 

    // load modules then do the setup
    require(['jquery', 'modules/entitiesList', 'modules/relations', 'modules/selection',
      'modules/structureTree', 'modules/validation'],
      function ($, EntitiesList, Relations, Selection, StructureTree, Validation) {
        // The function setupLayoutAndModules() is provided by layout.js, this
        // function assumes it was already loaded before it's called.
        setupLayoutAndModules(writer, EntitiesList, Relations, Selection, StructureTree, Validation);
        // Determine how to display.
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
  });
}
