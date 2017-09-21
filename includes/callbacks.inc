<?php

/**
 * @file
 * Menu Callbacks.
 */

/**
 * Menu callback for schema autocomplete in the admin settings form.
 *
 * @param string $query
 *   The query to filter the results by.
 */
function islandora_cwrc_writer_autocomplete_schema($query) {
  $tuque = islandora_get_tuque_connection();
  $schema_content_model = ISLANDORA_CWRC_WRITER_SCHEMA_CONTENT_MODEL;
  $query = <<<EOQ
PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
SELECT ?pid ?label
WHERE {
  ?pid fedora-model:label ?label ;
       fedora-model:hasModel <info:fedora/$schema_content_model> .
  FILTER(regex(?label, '$query') || regex(str(?pid), '$query' ))
}
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($query);
  $return = array();
  foreach ($results as $objects) {
    $return[$objects['pid']['value']] = t('@label (@pid)', array(
      '@label' => $objects['label']['value'],
      '@pid' => $objects['pid']['value'],
    ));
  }
  drupal_json_output($return);
}

/**
 * Menu callback that renders the CWRC-Writer for Demo Purposes.
 */
function islandora_cwrc_writer_demo() {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $unframed = array(
    'link' => array(
      '#theme' => 'link',
      '#text' => t('View in iFrame?'),
      '#path' => 'cwrc',
      '#options' => array(
        'attributes' => array(),
        'html' => TRUE,
      ),
    ),
    'cwrc' => array(
      '#theme' => 'islandora_cwrc_writer',
      '#documents' => islandora_cwrc_writer_get_objects_with_cwrc_datastream(),
    ),
  );
  $framed = array(
    'link' => array(
      '#theme' => 'link',
      '#text' => t('View unframed?'),
      '#path' => 'cwrc',
      '#options' => array(
        'attributes' => array(),
        'html' => TRUE,
        'query' => array('framed' => 'false'),
      ),
    ),
    'cwrc' => array(
      '#theme' => 'islandora_cwrc_writer_embedded',
    ),
  );
  $show_unframed = isset($_GET['framed']) && $_GET['framed'] === 'false';
  return array(
    'description' => array(
      '#prefix' => '<p>',
      '#suffix' => '</p>',
      '#markup' => t('This view of the CWRC-Writer can load any document which disseminates the CWRC datastream.'),
    ),
    'editor' => $show_unframed ? $unframed : $framed,
  );
}

/**
 * Renders the CWRC-Writer as a full HTML document.
 */
function islandora_cwrc_writer() {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  return array(
    '#type' => 'page',
    '#theme_wrappers' => array('islandora_cwrc_writer_iframe'),
    'content' => array(
      '#theme' => 'islandora_cwrc_writer',
      '#documents' => islandora_cwrc_writer_get_objects_with_cwrc_datastream(),
    ),
  );
}

/**
 * Renders the Note Editor.
 */
function islandora_cwrc_writer_note() {
  return array(
    '#type' => 'page',
    '#theme_wrappers' => array('islandora_cwrc_writer_iframe'),
    'content' => array(
      '#theme' => 'islandora_cwrc_writer_note',
    ),
  );
}

/**
 * We just mock the success response as we don't initialize anything.
 */
function islandora_cwrc_writer_cwrc_api_is_initialized() {
  drupal_json_output(array('success' => TRUE));
  drupal_exit();
}

/**
 * We just mock the success response as we don't initialize anything.
 */
function islandora_cwrc_writer_cwrc_api_initialize_cookie() {
  drupal_exit();
}
