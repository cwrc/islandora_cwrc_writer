<?php

/**
 * @file
 * Holds the schema and css upload form.
 *
 * Right now there is only this one step for Schema objects. If there is ever
 * more we will have to write undo steps as changing the controlGroup between
 * managed / redirect will cause problems.
 */

/**
 * The schema upload form.
 *
 * @param array $form
 *   The current form.
 * @param array $form_state
 *   The current form state.
 *
 * @return array
 *   The current form.
 */
function islandora_cwrc_writer_schema_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $rng_extensions = array('rng');
  $css_extensions = array('css');
  $pluploader_enabled = module_exists('islandora_pluploader');
  $title_display = 'before';
  $rng_file_prefix = NULL;
  $css_file_prefix = NULL;
  if ($pluploader_enabled) {
    $title_display = 'invisible';
    $rng_file_prefix = theme('form_element_label', array(
                            'element' => array(
                              '#id' => 'edit-rng-file',
                              '#title' => t('RNG File'),
                              '#title_display' => 'before',
                              '#required' => TRUE)));
    $css_file_prefix = theme('form_element_label', array(
                            'element' => array(
                              '#id' => 'edit-css-file',
                              '#title' => t('CSS File'),
                              '#title_display' => 'before',
                              '#required' => TRUE)));
  }
  return array(
    'label' => array(
      '#title' => t('Title'),
      '#description' => t('A unique title for this schema.'),
      '#type' => 'textfield',
      '#required' => TRUE,
      '#element_validate' => array('islandora_cwrc_writer_schema_upload_form_validate_label'),
    ),
    'location' => array(
      '#title' => t('Storage Location'),
      '#type' => 'radios',
      '#description' => t('<strong>Managed</strong> in this case, the RNG / CSS content will be stored in the repository and managed by this site.</br><strong>External</strong> in this case, the RNG / CSS content will be stored outside of the repository.'),
      '#options' => array(
        'managed' => t('Managed'),
        'external' => t('External'),
      ),
      '#default_value' => 'managed',
    ),
    'managed' => array(
      '#title' => t('Managed'),
      '#type' => 'fieldset',
      '#description' => t('A managed schema is useful in that it provides a stronger guarantee that it will be available; also, any changes to it can be tracked and managed over time within the repository.'),
      'rng_file' => array(
        // This is a hack to get around that pluploader doesn't
        // display #required.
        '#prefix' => $rng_file_prefix,
        '#title_display' => $title_display,
        // End hack.
        '#title' => t('RNG File'),
        '#type' => 'managed_file',
        '#description' => t('Select a file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $rng_extensions[0])),
        '#default_value' => isset($form_state['values']['rng_file']) ? $form_state['values']['rng_file'] : NULL,
        '#upload_location' => 'public://',
        '#upload_validators' => array(
          'file_validate_extensions' => $rng_extensions,
          // Assume it's specified in MB.
          'file_validate_size' => array($upload_size * 1024 * 1024),
        ),
      ),
      'css_file' => array(
        // This is a hack to get around that pluploader doesn't
        // display #required.
        '#prefix' => $css_file_prefix,
        '#title_display' => $title_display,
        // End hack.
        '#title' => t('CSS File'),
        '#type' => 'managed_file',
        '#description' => t('Select a file to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $css_extensions[0])),
        '#default_value' => isset($form_state['values']['rng_file']) ? $form_state['values']['rng_file'] : NULL,
        '#upload_location' => 'public://',
        '#upload_validators' => array(
          'file_validate_extensions' => $css_extensions,
          // Assume it's specified in MB.
          'file_validate_size' => array($upload_size * 1024 * 1024),
        ),
      ),
      '#states' => array(
        'visible' => array(
          ':input[name=location]' => array('value' => 'managed'),
        ),
      ),
    ),
    'external' => array(
      '#title' => t('External'),
      '#type' => 'fieldset',
      '#description' => t('An external schema is useful in that it can be shared among many objects, and changes to the file at the external location will be shared by all of them.'),
      'rng_url' => array(
        '#title' => t('RNG URL'),
        '#description' => t('A resolvable URL to a valid RNG file.'),
        '#type' => 'textfield',
        '#required' => FALSE,
        '#size' => 120,
        '#element_validate' => array(
          'islandora_cwrc_writer_schema_validate_url',
          'islandora_cwrc_writer_schema_validate_schema_mapping_url',
        ),
      ),
      'css_url' => array(
        '#title' => t('CSS URL'),
        '#description' => t('A resolvable URL to a valid CSS file.'),
        '#type' => 'textfield',
        '#required' => FALSE,
        '#size' => 120,
        '#element_validate' => array(
          'islandora_cwrc_writer_schema_validate_url',
        ),
      ),
      '#states' => array(
        'visible' => array(
          ':input[name=location]' => array('value' => 'external'),
        ),
      ),
    ),
  );
}

/**
 * Validates that the given label is unique among schema objects.
 *
 * @param array $element
 *   The element to validate.
 * @param array $form_state
 *   The drupal form state.
 * @param array $form
 *   The drupal form.
 */
function islandora_cwrc_writer_schema_upload_form_validate_label(array $element, array &$form_state, array $form) {
  $tuque = islandora_get_tuque_connection();
  $label = $element['#value'];
  $query = <<<EOT
PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
SELECT ?object WHERE {
  ?object <fedora-model:hasModel> <info:fedora/cwrc:schemaCModel> ;
          <fedora-model:label> "$label"
}
EOT;
  $count = $tuque->repository->ri->countQuery($query, 'sparql');
  if ($count !== 0) {
    form_error($element, t('The label "%label" is already in use.', array('%label' => $element['#value'])));
  }
}

/**
 * Check if the URL given is valid.
 *
 * @param array $element
 *   The element to validate.
 * @param array $form_state
 *   The drupal form state.
 * @param array $form
 *   The drupal form.
 */
function islandora_cwrc_writer_schema_validate_url(array $element, array &$form_state, array $form) {
  if ($form_state['values']['location'] == 'external') {
    if (filter_var($element['#value'], FILTER_VALIDATE_URL) === FALSE) {
      form_error($element, t('Not a valid URL.'));
    }
  }
}

/**
 * Check if the schema at the given url has a valid mapping.
 *
 * @param array $element
 *   The element to validate.
 * @param array $form_state
 *   The drupal form state.
 * @param array $form
 *   The drupal form.
 */
function islandora_cwrc_writer_schema_validate_schema_mapping_url(array $element, array &$form_state, array $form) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  if ($form_state['values']['location'] == 'external' && filter_var($element['#value'], FILTER_VALIDATE_URL) !== FALSE) {
    $filename = _islandora_cwrc_writer_schema_get_file_from_url($element['#value']);
    if ($filename === FALSE) {
      form_error($element, t('Failed to download RNG file.'));
    }
    else {
      $mapping = islandora_cwrc_writer_guess_schema_mapping_from_schema($filename);
      if ($mapping === FALSE) {
        form_error($element, t('The schema given does not match any valid CWRC-Writer mapping.'));
      }
      file_unmanaged_delete($filename);
    }
  }
}

/**
 * Helper function to fetch a a file from the given URL.
 *
 * @param string $url
 *   The URL to the file to download.
 *
 * @return string
 *   The filename of the downloaded file if successful, FALSE otherwise. The
 *   callee is responsible for deleting the file.
 */
function _islandora_cwrc_writer_schema_get_file_from_url($url) {
  $filename = file_create_filename("schema.rng", 'temporary://');
  $content = @file_get_contents($url);
  if ($content === FALSE) {
    file_unmanaged_delete($filename);
    return FALSE;
  }
  elseif (file_put_contents($filename, $content) === FALSE) {
    file_unmanaged_delete($filename);
    return FALSE;
  }
  return $filename;
}

/**
 * Validate the file fields as #element_validate doesn't work well with them.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_cwrc_writer_schema_upload_form_validate(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  if ($form_state['values']['location'] != 'managed') {
    return;
  }
  $file = file_load($form_state['values']['rng_file']);
  if ($file === FALSE) {
    form_set_error('managed][rng_file', t('@name field is required.', array('@name' => $form['managed']['rng_file']['#title'])));
  }
  else {
    $rng_path = drupal_realpath($file->uri);
    $mapping = islandora_cwrc_writer_guess_schema_mapping_from_schema($rng_path);
    if ($mapping === FALSE) {
      form_set_error('managed][rng_file', t('The schema given does not match any valid CWRC-Writer mapping.'));
    }
  }
  $file = file_load($form_state['values']['css_file']);
  if ($file === FALSE) {
    form_set_error('managed][css_file', t('@name field is required.', array('@name' => $form['managed']['css_file']['#title'])));
  }
}

/**
 * Adds the uploaded files to the ingestible object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_cwrc_writer_schema_upload_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  $object = islandora_ingest_form_get_object($form_state);
  $object->label = $form_state['values']['label'];
  if ($form_state['values']['location'] == 'managed') {
    _islandora_cwrc_writer_schema_create_managed($object, $form_state);
  }
  else {
    _islandora_cwrc_writer_schema_create_external($object, $form_state);
  }

}

/**
 * Creates a schema object where it's datastreams are managed.
 *
 * @param AbstractObject $object
 *   The object to attach the schema / css files to.
 * @param array $form_state
 *   The drupal form state.
 */
function _islandora_cwrc_writer_schema_create_managed(AbstractObject $object, array &$form_state) {
  if (empty($object['SCHEMA'])) {
    $ds = $object->constructDatastream('SCHEMA', 'M');
    $ds->mimetype = 'application/xml';
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['SCHEMA'];
  }
  $rng_file = file_load($form_state['values']['rng_file']);
  $rng_path = drupal_realpath($rng_file->uri);
  $ds->setContentFromFile($rng_path, FALSE);

  $mapping = islandora_cwrc_writer_guess_schema_mapping_from_schema($rng_path);
  islandora_cwrc_writer_set_schema_mapping($object, $mapping[ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP]);
  islandora_cwrc_writer_set_template_mapping($object, $mapping);

  if ($ds->label !== $rng_file->filename) {
    $ds->label = $rng_file->filename;
  }

  if (empty($object['CSS'])) {
    $ds = $object->constructDatastream('CSS', 'M');
    $ds->mimetype = 'text/css';
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['CSS'];
  }
  $css_file = file_load($form_state['values']['css_file']);
  $css_path = drupal_realpath($css_file->uri);
  $ds->setContentFromFile($css_path, FALSE);
  if ($ds->label !== $css_file->filename) {
    $ds->label = $css_file->filename;
  }
}

/**
 * Creates a schema object where it's datastreams are external.
 *
 * @param AbstractObject $object
 *   The object to attach the schema / css files to.
 * @param array $form_state
 *   The drupal form state.
 */
function _islandora_cwrc_writer_schema_create_external(AbstractObject $object, array &$form_state) {
  if (empty($object['SCHEMA'])) {
    $ds = $object->constructDatastream('SCHEMA', 'R');
    $ds->label = 'SCHEMA';
    $ds->mimetype = 'application/xml';
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['SCHEMA'];
  }
  $ds->url = $form_state['values']['rng_url'];

  // Its possible that we might not be able to download the file again even
  // though it passed the validation step, but for now we're ignoring it.
  $filename = _islandora_cwrc_writer_schema_get_file_from_url($ds->url);
  $mapping = islandora_cwrc_writer_guess_schema_mapping_from_schema($filename);
  islandora_cwrc_writer_set_schema_mapping($object, $mapping[ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP]);
  islandora_cwrc_writer_set_template_mapping($object, $mapping);
  
  file_unmanaged_delete($filename);

  if (empty($object['CSS'])) {
    $ds = $object->constructDatastream('CSS', 'R');
    $ds->label = 'CSS';
    $ds->mimetype = 'text/css';
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object['CSS'];
  }
  $ds->url = $form_state['values']['css_url'];
}
