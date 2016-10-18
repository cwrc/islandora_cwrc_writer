/*jshint browser: true*/
/*global jQuery, Drupal*/
/**
 * @file
 * Defines a function to override the load Document / Template dialog behavior.
 *
 * We do so to reduce the number of request required and also to remove the need
 * for a context specific menu callback to fetch the documents. As of now the
 * documents are passed as Drupal settings, so the caller of the theme has
 * control over which documents are listed, they don't have the additional
 * burden of defining a custom menu callback to do so.
 */
Drupal.CWRCWriter = Drupal.CWRCWriter || {};
Drupal.CWRCWriter.dialogManager = Drupal.CWRCWriter.dialogManager || {};
Drupal.CWRCWriter.dialogManager.filemanager = {
  showLoader: function ($, writer) {
    'use strict';
    return function () {
      var files, populateLoader, getTemplates;
      files = $('#files');
      files.css({borderColor: '#fff'});
      files.find('li').removeClass('selected');
      files.find('ul').html('<li class="unselectable last"><span class="loading" /></li>');
      $('#loaderDialog').dialog('open');
      populateLoader = function (data, columnIndex) {
        var formattedResults, last, d, i, uri, label;
        formattedResults = '';
        last = '';
        for (i = 0; i < data.length; i += 1) {
          d = data[i];
          if (i === data.length - 1) {
            last = 'last';
          } else {
            last = '';
          }
          if ($.isPlainObject(d)) {
            uri = d.path;
            label = d.name;
          } else {
            uri = d;
            label = d;
          }
          formattedResults += '<li class="unselectable ' + last + '" data-uri="' + uri + '">';
          formattedResults += '<span>' + label + '</span>';
          formattedResults += '</li>';
        }
        files.find('ul').eq(columnIndex).html(formattedResults);
      };
      getTemplates = function (callback) {
        writer.delegator.getTemplates(callback);
      };
      // Use the documents passed down by the theme.
      populateLoader(Drupal.settings.CWRCWriter.documents, 0);
      // Fetch the documents using CWRC-Delegator, which should fetch them from
      // Github.
      getTemplates(function (docs) {
        populateLoader(docs, 1);
      });
    };
  }
};
