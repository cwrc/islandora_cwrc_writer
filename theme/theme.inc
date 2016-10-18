<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Renders the CWRC-Writer as an iframe.
 */
function theme_islandora_cwrc_writer_embedded(array $variables) {
  $src = $variables['src'];
  $width = $variables['width'];
  $height = $variables['height'];
  return "<iframe src='$src' width='$width' height='$height' style='border: none;'></iframe>";
}

/**
 * Implements template_preprocess_islandora_cwrc_writer_iframe().
 */
function template_preprocess_islandora_cwrc_writer_iframe(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  // Show the content in full screen.
  $module_path = drupal_get_path('module', 'islandora_cwrc_writer');
  islandora_cwrc_record_new_resources_to_flag();
  drupal_add_css("$module_path/css/islandora_cwrc_writer.iframe.css");
  drupal_add_css("$module_path/css/islandora_cwrc_writer.writer.css");
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Implements template_process_islandora_cwrc_writer_iframe().
 */
function template_process_islandora_cwrc_writer_iframe(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  // Render out only those resources which are required for the CWRC-Writer.
  // Note that only specific CSS / JS files will get loaded. See the constant
  // ISLANDORA_CWRC_WRITER_RESOURCE_FLAG for more details. Note that this must
  // be last as the other renders above may include some scripts.
  $variables['styles']  = drupal_get_css(islandora_cwrc_writer_embedded_css(), TRUE);
  $variables['scripts'] = drupal_get_js('header', islandora_cwrc_writer_embedded_js());
  $variables['content'] = drupal_render($variables['page']['content']);
}

/**
 * Implements template_preprocess_islandora_cwrc_writer().
 */
function template_preprocess_islandora_cwrc_writer(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  // Start recording which resources should be rendered as part of the
  // CWRCWriter.
  islandora_cwrc_record_new_resources_to_flag();
  // Load the CWRC-Writer.
  $cwrc_writer_lib = libraries_load(ISLANDORA_CWRC_WRITER_LIB);
  $variables['title'] = isset($variables['title']) ? $variables['title'] : "CWRC-Writer v{$cwrc_writer_lib['version']}";
  // Add navigation to the CWRC-Writer header if relevant.
  if (count($variables['documents']) > 1) {
    $navigation_widget = islandora_cwrc_writer_document_navigation_widget($variables['documents']);
    array_unshift($variables['header'], $navigation_widget);
  }
  $variables['settings']['documents'] = $variables['documents'];
  $variables['settings']['schemaId'] = $variables['schema'];
  _template_preprocess_islandora_cwrc_writer_schema_settings($variables);
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Implements template_process_islandora_cwrc_writer().
 */
function template_process_islandora_cwrc_writer(array &$variables) {
  $module_path = drupal_get_path('module', 'islandora_cwrc_writer');
  // Override the CWRC-Writer/src/js/dialogs/fileManager.js
  // removed as the load document/template not longer used in the embedded
  //drupal_add_js("$module_path/js/dialogs/fileManager.js");
  drupal_add_js("$module_path/js/islandora_cwrc_writer.js");
  drupal_add_css("$module_path/css/islandora_cwrc_writer.base.css");
  // 2016-10-18 deprecated CSS
  //drupal_add_css("$module_path/css/islandora_cwrc_writer.writer.css");
  // The Widgets should be preprocessed last.
  drupal_add_js(array('CWRCWriter' => $variables['settings']), array('type' => 'setting'));
  islandora_cwrc_writer_preprocess_source_widget($variables);
  $variables['header'] = drupal_render($variables['header']);
  $variables['western_tabs'] = drupal_render($variables['western_tabs']);
  $variables['western_tabs_content'] = drupal_render($variables['western_tabs_content']);
  $variables['southern_tabs'] = drupal_render($variables['southern_tabs']);
  if (isset($variables['eastern_panel']) && !is_string($variables['eastern_panel'])) {
    $variables['eastern_panel'] = drupal_render($variables['eastern_panel']);
  }
}

/**
 * Based on the source type preprocess a widget for displaying it.
 */
function islandora_cwrc_writer_preprocess_source_widget(array &$variables) {
  if (empty($variables['sources'])) {
    return;
  }
  // We assume all source objects have the same type and only check the first.
  $jwplayer_enabled = module_exists('islandora_jwplayer');
  $image_annotation_enabled = module_exists('islandora_image_annotation');
  $source = islandora_object_load(reset($variables['sources']));
  $is_audio_source = in_array('islandora:sp-audioCModel', $source->models);
  $is_video_source = in_array('islandora:sp_videoCModel', $source->models);
  if ($is_audio_source && $jwplayer_enabled) {
    islandora_cwrc_writer_preprocess_audio_widget($variables);
  }
  elseif ($is_video_source && $jwplayer_enabled) {
    islandora_cwrc_writer_preprocess_video_widget($variables);
  }
  elseif ($image_annotation_enabled) {
    $mappings = variable_get('islandora_annotation_mappings', array());
    foreach ($source->models as $model) {
      if (isset($mappings[$model])) {
        islandora_cwrc_writer_preprocess_image_annotation_widget($variables);
        break;
      }
    }
  }
}

/**
 * Preprocess the theme variables to show audio source content.
 */
function islandora_cwrc_writer_preprocess_audio_widget(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  islandora_cwrc_record_new_resources_to_flag();
  $object = islandora_object_load(reset($variables['sources']));
  if (!isset($object['PROXY_MP3']) || !islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['PROXY_MP3'])) {
    return;
  }
  $audio_url = url("islandora/object/{$object->id}/datastream/PROXY_MP3/view", array('absolute' => TRUE));
  $audio_params = array(
    'pid' => $object->id,
    'url' => $audio_url,
    'mime' => 'audio/mpeg',
    'width' => '100%',
  );
  // Thumbnail.
  $has_thumbnail = isset($object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['TN']);
  if ($has_thumbnail) {
    $tn_url = url("islandora/object/{$object->id}/datastream/TN/view");
    $variables['islandora_thumbnail_img'] = theme('image', array(
      'title' => $object->label,
      'path' => $tn_url,
    ));
    $audio_params['tn'] = $tn_url;
  }
  $viewer = islandora_get_viewer($audio_params, 'islandora_audio_viewers', $object);
  $variables['params'] = $audio_params;
  // If a viewer is found use it.
  if ($viewer) {
    $variables['eastern_panel'] = $viewer;
  }
  // Otherwise link directly to the MP3, use the TN as a linked image if found.
  elseif (isset($variables['islandora_thumbnail_img'])) {
    $variables['eastern_panel'] = l($variables['islandora_thumbnail_img'], $audio_params['url'], array('html' => TRUE));
  }
  // Otherwise link directly to the MP3, no image to show.
  else {
    $variables['eastern_panel'] = l($object->label, $audio_params['url']);
  }
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Preprocess the theme variables to show video source content.
 */
function islandora_cwrc_writer_preprocess_video_widget(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  islandora_cwrc_record_new_resources_to_flag();
  $object = islandora_object_load(reset($variables['sources']));
  $is_chrome = stristr($_SERVER['HTTP_USER_AGENT'], 'chrome') !== FALSE;
  $viewer_dsid = ($is_chrome && isset($object['OGG'])) ? 'OGG' : 'MP4';
  if (!isset($object[$viewer_dsid]) || !islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$viewer_dsid])) {
    return;
  }
  $video_params = array(
    'pid' => $object->id,
    'mime' => 'video/mp4',
    'url' => url("islandora/object/{$object->id}/datastream/$viewer_dsid/view"),
    'width' => "100%",
  );
  // Thumbnail.
  if (isset($object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['TN'])) {
    $video_params += array(
      'tn' => url("islandora/object/{$object->id}/datastream/TN/view", array('absolute' => TRUE)),
    );
  }
  $viewer = islandora_get_viewer($video_params, 'islandora_video_viewers', $object);
  $variables['params'] = $video_params;
  if ($viewer) {
    $variables['eastern_panel'] = $viewer;
  }
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Preprocess the theme variables to show the image annotation widget.
 */
function islandora_cwrc_writer_preprocess_image_annotation_widget(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $object = islandora_object_load(reset($variables['sources']));
  // Add the tab for image annotations.
  $variables['western_tabs']['#links']['image_annotations'] = array(
    'title' => t('Image Annotations'),
    'fragment' => 'image-annotations',
    'href' => '',
    'external' => TRUE,
  );
  // Add the tab for image annotations.
  $variables['western_tabs_content'] = array(
    '#prefix' => '<div id="image-annotations" class="tabWithLayout">',
    '#theme' => 'islandora_image_annotation_list',
    '#object' => $object,
    '#suffix' => '</div>',
  );
  // Add image annotation widget.
  $variables['eastern_panel'] = array(
    '#theme' => 'islandora_cwrc_writer_image_annotation',
    '#documents' => $variables['documents'],
    '#sources' => $variables['sources'],
  );
}

/**
 * Implements template_preprocess_islandora_cwrc_writer_note().
 */
function template_preprocess_islandora_cwrc_writer_note(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  islandora_cwrc_record_new_resources_to_flag();
  // Load the CWRC-Writer.
  libraries_load(ISLANDORA_CWRC_WRITER_LIB);
  $module_path = drupal_get_path('module', 'islandora_cwrc_writer');
  // We need to use the same default as the main CWRC-Writer.
  $variables['settings']['schemaId'] = $variables['schema'];
  _template_preprocess_islandora_cwrc_writer_schema_settings($variables);
  // Override CWRC-Writer Settings / startup behaviour.
  drupal_add_js(array('CWRCWriter' => $variables['settings']), array('type' => 'setting'));
  drupal_add_js("$module_path/js/islandora_cwrc_writer_note.js");
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Implements template_preprocess_islandora_cwrc_writer_image_annotation().
 */
function template_preprocess_islandora_cwrc_writer_image_annotation(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  islandora_cwrc_record_new_resources_to_flag();
  $object = islandora_object_load(reset($variables['sources']));
  // Set preprocessor variables, for use in JavaScript.
  $variables['dialog_box'] = array(
    '#theme' => 'islandora_image_annotation_dialog',
    '#object' => $object,
  );
  // Requires the jQuery UI Accordion Widget.
  drupal_add_library('system', 'ui.accordion');
  $variables['text_image_link_dialog_box'] = array(
    '#theme' => 'islandora_cwrc_writer_text_image_link_dialog',
    '#object' => $object,
  );
  $variables['text_image_link_button'] = array(
    '#theme' => 'islandora_cwrc_writer_text_image_link_button',
  );
  // The canvas is where the image and it's annotations get rendered.
  $variables['canvas'] = array(
    '#theme' => 'islandora_image_annotation_canvas',
    '#object' => $object,
  );
  // Drupal Settings.
  $settings = array(
    'pid' => $object->id,
  );
  drupal_add_js(array('islandoraImageAnnotation' => $settings), 'setting');
  // Include required Javascript libraries.
  $image_annotation_module_path = drupal_get_path('module', 'islandora_image_annotation');
  drupal_add_js("$image_annotation_module_path/lib/jquery/ContextMenu/jquery.contextMenu.js");
  drupal_add_js("$image_annotation_module_path/lib/jquery/miniColors/jquery.miniColors.js");
  drupal_add_js("$image_annotation_module_path/lib/jquery/jquery.rdfquery.rdfa.min-1.1.js");
  drupal_add_js("$image_annotation_module_path/lib/raphael/raphael.js");
  drupal_add_js("$image_annotation_module_path/lib/raphael/scale.raphael.js");
  drupal_add_js("$image_annotation_module_path/lib/uuid.js");
  drupal_add_js("$image_annotation_module_path/lib/he.js");
  drupal_add_js("$image_annotation_module_path/js/islandora_image_annotation_utils.js");
  drupal_add_js("$image_annotation_module_path/js/rdf.js");
  drupal_add_js("$image_annotation_module_path/js/islandora_image_annotation.js");
  drupal_add_js("$image_annotation_module_path/js/islandora_image_annotation_buttons.js");
  drupal_add_css("$image_annotation_module_path/lib/jquery/ContextMenu/jquery.contextMenu.css");
  drupal_add_css("$image_annotation_module_path/lib/jquery/miniColors/jquery.miniColors.css");
  drupal_add_css("$image_annotation_module_path/css/islandora_image_annotation.base.css");
  // And our own customizations.
  $module_path = drupal_get_path('module', 'islandora_cwrc_writer');
  drupal_add_js("$module_path/js/islandora_cwrc_writer_image_annotation.js");
  drupal_add_js("$module_path/js/islandora_cwrc_writer_image_annotation_dialog.js");
  drupal_add_css("$module_path/css/islandora_cwrc_writer.image_annotation.css");
  // To allow for use to dynamically change the image annotation with out doing
  // a page refresh.
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  $settings = array(
    'documents' => $variables['documents'],
    'sources' => $variables['sources'],
  );
  foreach ($variables['sources'] as $source) {
    $source = islandora_object_load($source);
    $settings[$source->id] = array(
      'islandoraImageAnnotation' => array(
        'pid' => $source->id,
      ),
      'islandoraImageAnnotationDialog' => array(
        'pid' => $source->id,
        'categories' => islandora_image_annotation_get_categories($source),
        'enforceTaxonomy' => variable_get('islandora_image_annotation_enforce_taxonomy', TRUE),
        'allowEntityLinking' => variable_get('islandora_image_annotation_use_entity_tagging', TRUE),
        'useTitleVocabulary' => variable_get('islandora_image_annotation_use_title_vocabulary', FALSE),
        'strokeWidths' => variable_get('islandora_image_annotation_stroke_width', '.3 .6 .9 1.2 1.5'),
        'canChooseColour' => !variable_get('islandora_image_annotation_choose_color', TRUE),
        'dialog' => array(
          'dialogClass' => 'islandora-image-annotation',
        ),
      ),
      'islandoraImageAnnotationList' => array(
        'pid' => $source->id,
        'editable' => user_access(ISLANDORA_IMAGE_ANNOTATION_CREATE),
        'annotations' => islandora_image_annotation_get_annotations($source),
      ),
    );
  }
  drupal_add_js(array('islandoraCWRCWriterImageAnnotation' => $settings), 'setting');
  drupal_add_js("$module_path/js/islandora_cwrc_writer_image_annotation_hack_settings.js", array(
    'type' => 'file',
    // We can't access the fragment identifier server side, and it's used by the
    // CWRC-Writer to select the current document. So we hack the settings of
    // the Image Annotation Module, before Drupal.attachBehaviours() runs, aka
    // before jQuery.ready() is called.
    'weight' => 100,
  ));
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Implements template_process_islandora_cwrc_writer_image_annotation().
 */
function template_process_islandora_cwrc_writer_image_annotation(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  islandora_cwrc_record_new_resources_to_flag();
  $image_annotation_module_path = drupal_get_path('module', 'islandora_image_annotation');
  // All dialog's follow this format to get the CSS to work out.
  $variables['dialog_box'] = drupal_render($variables['dialog_box']);
  $variables['text_image_link_dialog_box'] = drupal_render($variables['text_image_link_dialog_box']);
  $variables['text_image_link_button'] = drupal_render($variables['text_image_link_button']);
  $variables['canvas'] = drupal_render($variables['canvas']);
  // Change the dialog class so the boxes look correct.
  drupal_add_js(array(
    'islandoraImageAnnotationDialog' => array(
      'dialog' => array(
        'dialogClass' => 'splitButtons',
        'position' => array(
          'my' => "left top",
          'at' => "left top",
          'of' => '#cwrc_wrapper',
        ),
      ),
    )), 'setting');
  drupal_add_css("$image_annotation_module_path/css/islandora_image_annotation.dialog.css");
  drupal_add_css("$image_annotation_module_path/css/islandora_image_annotation.list.css");
  drupal_add_css("$image_annotation_module_path/css/islandora_image_annotation.canvas.css");
  islandora_cwrc_flag_recorded_resources();
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_cwrc_writer_text_image_link_button(array &$variables) {
  $variables['icon'] = array(
    '#theme' => 'image',
    '#alt' => t('Tag Text Annotation'),
    '#path' => drupal_get_path('module', 'islandora_cwrc_writer') . '/images/img_text.png',
    '#attributes' => array(
      'class' => 'mceIcon',
    ),
  );
}

/**
 * Implements template_process_theme().
 */
function template_process_islandora_cwrc_writer_text_image_link_button(array &$variables) {
  $variables['icon'] = drupal_render($variables['icon']);
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_cwrc_writer_text_image_link_dialog(array &$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'theme/theme');
  template_preprocess_islandora_image_annotation_dialog($variables);
}

/**
 * Implements template_process_theme().
 */
function template_process_islandora_cwrc_writer_text_image_link_dialog(array &$variables) {
  template_process_islandora_image_annotation_dialog($variables);
}

/**
 * Helper function used to preprocess the schemas section of the settings.
 *
 * This should be called in the template_preprocess hooks of anything that
 * renders the CWRC-Writer.
 *
 * @param array $variables
 *   The same $variables array passed to a CWRC-Writer theme preprocess
 *   function.
 */
function _template_preprocess_islandora_cwrc_writer_schema_settings(array &$variables) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $variables['settings']['schemas'] = islandora_cwrc_writer_get_schemas();
}
