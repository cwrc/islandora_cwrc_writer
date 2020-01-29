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

// cookie code from https://github.com/carhartl/jquery-cookie/blob/master/src/jquery.cookie.js
(function ($) {

	var pluses = /\+/g;

	function encode(s) { return config.raw ? s : encodeURIComponent(s); }

	function decode(s) { return config.raw ? s : decodeURIComponent(s); }

	function stringifyCookieValue(value) { return encode(config.json ? JSON.stringify(value) : String(value)); }

	function parseCookieValue(s) {
		if (s.indexOf('"') === 0) {
			s = s.slice(1, -1).replace(/\\"/g, '"').replace(/\\\\/g, '\\');
		}

		try {
			s = decodeURIComponent(s.replace(pluses, ' '));
			return config.json ? JSON.parse(s) : s;
		} catch(e) {}
	}

	function read(s, converter) {
		var value = config.raw ? s : parseCookieValue(s);
		return $.isFunction(converter) ? converter(value) : value;
	}

	var config = $.cookie = function (key, value, options) {
		if (arguments.length > 1 && !$.isFunction(value)) {
			options = $.extend({}, config.defaults, options);

			if (typeof options.expires === 'number') {
				var days = options.expires, t = options.expires = new Date();
				t.setMilliseconds(t.getMilliseconds() + days * 864e+5);
			}

			return (document.cookie = [
				encode(key), '=', stringifyCookieValue(value),
				options.expires ? '; expires=' + options.expires.toUTCString() : '',
				options.path    ? '; path=' + options.path : '',
				options.domain  ? '; domain=' + options.domain : '',
				options.secure  ? '; secure' : ''
			].join(''));
        }
        
		var result = key ? undefined : {},
			cookies = document.cookie ? document.cookie.split('; ') : [],
			i = 0,
			l = cookies.length;

		for (; i < l; i++) {
			var parts = cookies[i].split('='),
				name = decode(parts.shift()),
				cookie = parts.join('=');

			if (key === name) {
				result = read(cookie, value);
				break;
			}

			if (!key && (cookie = read(cookie)) !== undefined) {
				result[name] = cookie;
			}
		}

		return result;
	};

	config.defaults = {};

	$.removeCookie = function (key, options) {
		$.cookie(key, '', $.extend({}, options, { expires: -1 }));
		return !$.cookie(key);
	};

}(jQuery));


(function ($) {
  'use strict';
  
  var layoutParentId = 'cwrc_wrapper'; // specified in islandora-cwrc-writer.tpl.php
  
  function cwrcWriterInit($, Writer, Dialogs) {
      'use strict';
      var writer, config;
      
      config = Drupal.settings.CWRCWriter;

      // convert old schemas to new schema format
      config.schema = {
        schemas: []
      };
      for (var key in config.schemas) {
        var schemaEntry = config.schemas[key];
        config.schema.schemas.push({
          pid: schemaEntry.pid, // islandora specific
          aliases: schemaEntry.aliases, // islandora specific
          id: key,
          name: schemaEntry.name,
          schemaMappingsId: schemaEntry.schemaMappingsId,
          xmlUrl: [schemaEntry.url],
          cssUrl: [schemaEntry.cssUrl]
        })
      }
      delete config.schemas;
      
      if (config.initial_mode == 'edit') {
          config.buttons1 += ',saveexitbutton';
      }
      config.buttons1 += ',|,fullscreen';
      config.container = config.id || layoutParentId;
      config.modules = {
          west: [
            {id: 'structure', title: 'Markup'},
            {id: 'entities', title: 'Entities'},
            {id: 'nerve', title: 'NERVE', config: {
              'nerveUrl': 'https://nerve.services.cwrc.ca'
            }}
          ],
          south: [
            {id: 'selection', title: 'Selection'},
            {id: 'validation', title: 'Validation', config: {
              'validationUrl': 'https://validator.services.cwrc.ca/validator/validate.html'
            }}
          ],
          east: [
            {id: 'imageViewer', title: 'Image Viewer'}
          ]
      };
      
      config.entityLookupDialogs = Dialogs;

      var baseUrl = config.baseUrl;
      config.storageDialogs = {
          save: function(writer) {
              var docId = writer.currentDocId;
              var docText = writer.converter.getDocumentContent(true);
              $.ajax({
                  url : baseUrl+'editor/documents/'+docId,
                  type: 'PUT',
                  dataType: 'json',
                  data: {'doc':docText, 'schema':writer.schemaManager.getCurrentSchema()['pid']},
                  success: function(data, status, xhr) {
                      writer.dialogManager.show('message', {
                          title : 'Document Saved',
                          msg : docId + ' was saved successfully.'
                      });
                      writer.event('documentSaved').publish();
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
                      url: baseUrl+'editor/documents/'+docId,
                      type: 'GET',
                      success: function(doc, status, xhr) {
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
      
      writer.utilities.addCSS('css/bootstrap.css');
      writer.utilities.addCSS('css/islandora-overrides.css');
      
      window.addEventListener('beforeunload', function() {
        // remove cookie specifying which collection to save new entities to (which was added by cwrc-entity-management-forms)
        $.removeCookie('cwrc-entity-collection', { path: '/' })
      });
      /**
       * Overwrite the writer.saveAndExit
       */
      writer.saveAndExit = function() {
          var docText = writer.converter.getDocumentContent(true);
          $.ajax({
              url : baseUrl+'editor/documents/'+writer.currentDocId,
              type: 'PUT',
              dataType: 'json',
              data: {'doc':docText, 'schema':writer.schemaManager.getCurrentSchema()['pid']},
              success: function(data, status, xhr) {
                  writer.editor.isNotDirty = true;

                  $.ajax({
                      url: Drupal.settings.basePath+'islandora/rest/v1/object/'+writer.currentDocId+'/lock',
                      type: 'DELETE',
                      success: function(data, status, xhr) {
                          window.location = Drupal.settings.basePath+'islandora/object/'+writer.currentDocId
                      },
                      error: function() {
                          displayError(xhr, writer.currentDocId);
                      }
                  })
              },
              error: function(xhr, status, error) {
                  displayError(xhr, writer.currentDocId);
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

      // Hold onto a reference for safe keeping.
      Drupal.CWRCWriter.writer = writer;
      writer.event('writerInitialized').subscribe(function (writer) {

          if (typeof config.initial_mode !== 'undefined') {
              if (config.initial_mode == 'annotate') {
//                writer.layoutManager.activateAnnotator();
                writer.layoutManager.showModule('entities');
                
              }
              else if (config.initial_mode == 'read') {
//                writer.layoutManager.activateReader();
                writer.layoutManager.showModule('structure');
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
              setTimeout(writer.layoutManager.resizeAll.bind(writer.layoutManager), 500);
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
	  if ( height < 850) {
	    height = 850;
	  }
          $('#'+layoutParentId).height(height);
        }
        
        cwrcWriterInit($, Drupal.CWRCWriter, Drupal.CWRCWriterDialogs);
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
