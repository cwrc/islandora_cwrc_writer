<?php

/**
 * @file
 * Miscellaneous helper functions.
 */

define('ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR', 'islandora_cwrc_record_resources');
define('ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR_COUNT', 'islandora_cwrc_record_resources_count');

/**
 * Checks if the given JS / CSS resource options are needed for the CWRC-Writer.
 *
 * @param array $options
 *   The options array as it would be passed to either drupal_add_js() or
 *   drupal_add_css().
 *
 * @return bool
 *   TRUE if the given resource options indicate the resource is needed for the
 *   CWRC-Writer, FALSE otherwise.
 */
function islandora_cwrc_writer_is_cwrc_writer_resource(array $options) {
  return isset($options[ISLANDORA_CWRC_WRITER_RESOURCE_FLAG]) ? $options[ISLANDORA_CWRC_WRITER_RESOURCE_FLAG] : FALSE;
}

/**
 * Flags the given JS / CSS resource for use by the CWRC-Writer.
 *
 * @param array $options
 *   The options array as it would be passed to either drupal_add_js() or
 *   drupal_add_css()
 */
function islandora_cwrc_writer_flag_as_cwrc_writer_resource(array &$options) {
  $options[ISLANDORA_CWRC_WRITER_RESOURCE_FLAG] = TRUE;
}

/**
 * Starts recording newly added js / css files to add to the CWRC-Writer.
 *
 * It does so by keeping track of the resource in current use and comparing it
 * to the ones defined later, when the stop recording function is called.
 *
 * This function in a helper function that allows us to frame a section of code
 * and allow any resources added in that section to be made available to the
 * CWRC-Writer.
 *
 * It comes in handy if you want to include some external library via some
 * helper function with out explicitly including all the resources manually with
 * the ISLANDORA_CWRC_WRITER_RESOURCE_FLAG set.
 */
function islandora_cwrc_record_new_resources_to_flag() {
  // This function can be called multiple times, we should keep track such that
  // we don't loose previously stored values.
  $count = &drupal_static(ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR_COUNT, 0);
  if ($count == 0) {
    drupal_static(ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR, array(
      'js' => drupal_static('drupal_add_js', array()),
      'css' => drupal_static('drupal_add_css', array()),
    ));
  }
  $count++;
}

/**
 * Stops recording newly added resources and flags them for the CWRC-Writer.
 */
function islandora_cwrc_flag_recorded_resources() {
  // We only want to flag the resources on the last call for the record the
  // resources.
  $count = &drupal_static(ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR_COUNT, 0);
  if ($count == 1) {
    $before = drupal_static(ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR, array(
      'js' => array(),
      'css' => array(),
    ));
    $js = &drupal_static('drupal_add_js', array());
    $css = &drupal_static('drupal_add_css', array());
    $after = array(
      'js' => &$js,
      'css' => &$css,
    );
    foreach (array('js', 'css') as $type) {
      $files = array_diff(array_keys($after[$type]), array_keys($before[$type]));
      foreach ($files as $file) {
        islandora_cwrc_writer_flag_as_cwrc_writer_resource($after[$type][$file]);
      }
    }
    // Unset.
    drupal_static_reset(ISLANDORA_CWRC_WRITER_RECORD_RESOURCES_STATIC_VAR);
  }
  $count--;
}

/**
 * Fetches the paths for all the drupal required javascript files.
 *
 * @return array
 *   An array of paths to each of the system required javascript files, relative
 *   to DRUPAL_ROOT.
 */
function _islandora_cwrc_writer_drupal_required_js_files() {
  // By default in drupal_add_js() defined in DRUPAL_ROOT/includes/common.inc
  // settings, drupal.js, jquery.js, and jquery.once.js is included. We get the
  // library definitions for each of the jQuery libraries as a module might
  // overwrite those files, so we can't assume an location.
  $lib_jquery = drupal_get_library('system', 'jquery');
  // jQuery Update has the file path in a different location then the default.
  if (module_exists('jquery_update')) {
    $info = reset($lib_jquery['js']);
    $jquery = array($info['data']);
  }
  // Devel and Drupal both use the array key value to store the file location.
  else {
    $jquery = array_keys($lib_jquery['js']);
  }
  $lib_jquery_once = drupal_get_library('system', 'jquery.once');
  return array_merge(array('settings', 'misc/drupal.js'), $jquery, array_keys($lib_jquery_once['js']));
}

/**
 * Gets the javascript files, settings, etc, for the CWRC-Writer when embedded.
 *
 * @return array
 *   The current array of JavaScript files, settings, and in-line code,
 *   including Drupal defaults, anything previously added with calls to
 *   drupal_add_js() that had 'cwrc_writer' => TRUE as an option.
 */
function islandora_cwrc_writer_embedded_js() {
  // Just experimenting here, we can't rely on a method of adding only that
  // which we need as what we need may be shared by other drupal processes, and
  // could have been added earlier so this algorithm will miss those cases.
  $js = drupal_add_js();
  $required_drupal_js_files = _islandora_cwrc_writer_drupal_required_js_files();
  $required_drupal_js_files = array_fill_keys($required_drupal_js_files, TRUE);
  $drupal_js = array_intersect_key($js, $required_drupal_js_files);
  $cwrc_js  = array_filter($js, 'islandora_cwrc_writer_is_cwrc_writer_resource');
  return $drupal_js + $cwrc_js;
}

/**
 * Gets the javascript files, settings, etc, for the CWRC-Writer when embedded.
 *
 * @return array
 *   The current array of JavaScript files, settings, and in-line code,
 *   including Drupal defaults, anything previously added with calls to
 *   drupal_add_js() that had 'cwrc_writer' => TRUE as an option.
 */
function islandora_cwrc_writer_embedded_css() {
  return array_filter(drupal_add_css(), 'islandora_cwrc_writer_is_cwrc_writer_resource');
}

/**
 * Gets the default properties to use with the CWRC-Writer's writer delegate.
 *
 * @return array
 *   The required properties expected to be placed in Drupal.settings.CWRCWriter
 *   so that the CWRC-Writer can function correctly.
 */
function islandora_cwrc_writer_default_config() {
  global $base_url;
  $cwrc_writer_path = libraries_get_path(ISLANDORA_CWRC_WRITER_LIB);
  return array(
    'project' => 'cwrc',
    // We need trailing slashes on the urls for CWRC to work.
    'cwrcRootUrl' => file_create_url("$cwrc_writer_path/src/"),
    // The url in which we provide services for.
    'baseUrl' => islandora_cwrc_writer_get_absolute_url('cwrc/'),
    // XML + RDF.
    'mode' => 0,
    'allowOverlap' => TRUE,
    // Custom to Islandora this allows use to populate the 'Documents' section
    // of the load dialog. The #documents theme variable will populate this at
    // run time.
    'documents' => array(),
    'cwrcDialogs' => array(
      'cwrcApiUrl' => islandora_cwrc_writer_get_absolute_url(variable_get('islandora_cwrc_writer_cwrc_api_url', 'cwrc/api')),
      'geonameUrl' => islandora_cwrc_writer_get_absolute_url(variable_get('islandora_cwrc_writer_geo_names_url', 'geonames')),
      'googleGeocodeUrl' => islandora_cwrc_writer_get_absolute_url(variable_get('islandora_cwrc_writer_google_geocode_url', 'http://maps.googleapis.com/maps/api/geocode/xml'), array('external' => TRUE)),
      'viafUrl' => islandora_cwrc_writer_get_absolute_url(variable_get('islandora_cwrc_writer_viaf_url', 'viaf')),
      'schemas' => array(
        'person' => 'http://cwrc.ca/schemas/entities.rng',
        'organization' => 'http://cwrc.ca/schemas/entities.rng',
        'place' => 'http://cwrc.ca/schemas/entities.rng',
      ),
      'repositoryBaseObjectUrl' => islandora_cwrc_writer_get_absolute_url(variable_get('islandora_cwrc_writer_repository_base_object_url', '')),
    ),
    // This associative array of schema documents requires the PID of the schema
    // object to be used as each entries key.
    'schemas' => array(
      // These are all required objects for this module. They must have the
      // index of (tei, events, biography, writing) as the CWRC-Writer is
      // hardcoded to expect these schema's to be present. Dialog boxes will
      // break if they do not have these indexes.
      'cwrc:teiSchema' => array(
        'pid' => 'cwrc:teiSchema',
        'name' => 'CWRC Basic TEI Schema',
        'url' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:teiSchema/datastream/SCHEMA/view'),
        'cssUrl' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:teiSchema/datastream/CSS/view'),
        'aliases' => array(
          'http://cwrc.ca/schema/CWRC-TEIBasic.rng',
          'http://cwrc.ca/schemas/cwrc_tei_lite.rng',
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/cwrc:teiSchema/datastream/SCHEMA/view",
        ),
        'schemaMappingsId' => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_TEI,
      ),
      'cwrc:eventsSchema' => array(
        'pid' => 'cwrc:eventsSchema',
        'name' => 'CWRC Events Schema',
        'url' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:eventsSchema/datastream/SCHEMA/view'),
        'cssUrl' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:eventsSchema/datastream/CSS/view'),
        'aliases' => array(
          'http://cwrc.ca/schemas/orlando_event_v2.rng',
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/cwrc:eventsSchema/datastream/SCHEMA/view",
        ),
        'schemaMappingsId' => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ),
      'cwrc:biographySchema' => array(
        'pid' => 'cwrc:biographySchema',
        'name' => 'CWRC Biography Schema',
        'url' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:biographySchema/datastream/SCHEMA/view'),
        'cssUrl' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:biographySchema/datastream/CSS/view'),
        'aliases' => array(
          'http://cwrc.ca/schemas/orlando_biography_v2.rng',
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/cwrc:biographySchema/datastream/SCHEMA/view",
        ),
        'schemaMappingsId' => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ),
      'cwrc:writingSchema' => array(
        'pid' => 'cwrc:writingSchema',
        'name' => 'CWRC Writing Schema',
        'url' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:writingSchema/datastream/SCHEMA/view'),
        'cssUrl' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:writingSchema/datastream/CSS/view'),
        'aliases' => array(
          'http://cwrc.ca/schemas/orlando_writing_v2.rng',
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/cwrc:writingSchema/datastream/SCHEMA/view",
        ),
        'schemaMappingsId' => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ),
      'cwrc:entrySchema' => array(
        'pid' => 'cwrc:entrySchema',
        'name' => 'CWRC Entry Schema',
        'url' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:entrySchema/datastream/SCHEMA/view'),
        'cssUrl' => islandora_cwrc_writer_get_absolute_url('islandora/object/cwrc:entrySchema/datastream/CSS/view'),
        'aliases' => array(
          'http://cwrc.ca/schemas/cwrc_entry.rng',
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/cwrc:entrySchema/datastream/SCHEMA/view",
        ),
        'schemaMappingsId' => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ENTRY,
      ),
    ),
  );
}

/**
 * A Drupal renderable array of the header content.
 *
 * This a <div> that will be populated by CWRC-Writer on initialization.
 *
 * @return array
 *   A renderable array of the default header of the CWRC-Writer.
 */
function islandora_cwrc_writer_default_header() {
  return array(
    'header_buttons' => array(
      '#markup' => '<div id="headerButtons"></div>',
    ),
  );
}

/**
 * A Drupal renderable array of the default tabs in the western panel.
 *
 * This includes the panels expected by the CWRC-Writer without any
 * customization by islandora.
 *
 * Each link should be of the form <a href="#identifer" /> for the tabs to work.
 *
 * If you choose to use your own tabs, you must still have the 'structure' tab.
 * For the CWRC-Writer to function.
 *
 * @return array
 *   The renderable array of the default tabs to be displayed in the western
 *   panel.
 */
function islandora_cwrc_writer_default_western_tabs() {
  return array(
    '#theme' => 'links',
    '#links' => array(
      'entities' => array(
        'title' => t('Entities'),
        'fragment' => 'entities',
        'href' => '',
        'external' => TRUE,
      ),
      'structure' => array(
        'title' => t('Structure'),
        'fragment' => 'structure',
        'href' => '',
        'external' => TRUE,
      ),
      'relations' => array(
        'title' => t('Relations'),
        'fragment' => 'relations',
        'href' => '',
        'external' => TRUE,
      ),
    ),
  );
}

/**
 * A Drupal renderable array of the default tabs in the southern panel.
 *
 * This includes the panels expected by the CWRC-Writer without any
 * customization by islandora.
 *
 * Each link should be of the form <a href="#identifer" /> for the tabs to work.
 *
 * If you choose to use your own tabs, you must include both validation and
 * selection, for the CWRC-Writer to function.
 *
 * @return array
 *   The renderable array of the default tabs to be displayed in the western
 *   panel.
 */
function islandora_cwrc_writer_default_southern_tabs() {
  return array(
    '#theme' => 'links',
    '#links' => array(
      'validation' => array(
        'title' => t('Validation'),
        'fragment' => 'validation',
        'href' => '',
        'external' => TRUE,
      ),
      'selection' => array(
        'title' => t('Selection'),
        'fragment' => 'selection',
        'href' => '',
        'external' => TRUE,
      ),
    ),
  );
}

/**
 * Gets the default theme variables for the theme 'islandora_cwrc_writer'.
 *
 * Since we have many wrapper themes for the CWRC-Writer in which we want to
 * expose the CWRC-Writer configuration though we wrap the defaults so they can
 * be easily exposed by those wrapper themes.
 *
 * @return array
 *   The default theme properties for the CWRC-Writer.
 */
function islandora_cwrc_writer_default_theme_variables() {
  return array(
    // The Title of the CWRC-Writer when displayed, it will be set to
    // CWRC-Writer v#, if none is provided.
    'title' => NULL,
    // This field isn't used by this theme itself, but instead exists so that
    // modules that extend this theme have access to the 'owner' object in
    // which the 'documents' belong to.
    'object' => NULL,
    // The PID of the schema object to use for validating the given documents
    // (optional).
    'schema' => NULL,
    // An array of PIDs for the Objects whose content can be loaded into the
    // CWRC-Writer text editor. Currently this module is hard coded to use
    // the CWRC datastream of these objects. This variables is required.
    'documents' => array(),
    // An array of PIDs for the Objects whose content represents the source
    // material that the 'documents' where derived from. Currently this module
    // supports a single video / audio file or multiple page files from a book.
    'sources' => array(),
    // Settings will become Drupal.settings.CWRCWriter, these are used to
    // configure the editor.
    'settings' => islandora_cwrc_writer_default_config(),
    // A Drupal renderable array which will be displayed after the title in
    // the CWRC-Writer editor.
    'header' => islandora_cwrc_writer_default_header(),
    // A Drupal renderable array expected to be links which representing the
    // tabs to be displayed in the western panel.
    'western_tabs' => islandora_cwrc_writer_default_western_tabs(),
    // The default tabs are populated by the CWRCWriter itself.
    'western_tabs_content' => array(),
    // CWRC-Writer does not define any default tabs for the eastern panel,
    // therefore we can embed any content we wish, it does not have to be
    // tab panels. It's expecting a renderable array.
    'eastern_panel' => NULL,
    // A Drupal renderable array expected to be links which representing the
    // tabs to be displayed in the southern panel.
    'southern_tabs' => islandora_cwrc_writer_default_southern_tabs(),
    // Additional configuration options to pass through to the CWRC-Writer, so
    // that default ones don't have to be overridden necessarily.
    'config' => array(),
  );
}

/**
 * Defines a small widget composed of a select field and directional navigation.
 *
 * This is used in the CWRC-Writer header when more than one document is
 * defined.
 *
 * @param array $documents
 *   Expected to be an array of PIDs referring to the objects whose content can
 *   be loaded into the CWRC-Writer.
 *
 * @return array
 *   A renderable array of representing the navigation widget.
 */
function islandora_cwrc_writer_document_navigation_widget(array $documents) {
  $count = 1;
  $options = array();
  foreach ($documents as $pid) {
    $options[$pid] = "Seq# {$count}";
    $count++;
  }
  return count($options) == 0 ? NULL : array(
    'select' => array(
      '#id' => 'islandora-cwrc-document-select',
      '#theme' => 'select',
      '#options' => $options,
      '#prefix' => '<div id="islandora-cwrc-document-select-wrapper">',
      '#suffix' => '</div>',
    ),
    'nav' => array(
      '#theme' => 'links',
      '#attributes' => array('id' => 'islandora-cwrc-document-nav'),
      '#links' => array(
        'prev' => array(
          'title' => '',
          'fragment' => 'prev',
          'href' => '',
          'external' => TRUE,
          'attributes' => array('class' => 'disabled'),
        ),
        'next' => array(
          'title' => '',
          'fragment' => 'next',
          'href' => '',
          'external' => TRUE,
        ),
      ),
    ),
  );
}

/**
 * Gets all objects with a CWRC datastream, used for the demo CWRC-Writer page.
 *
 * @return array
 *   The objects with a CWRC datastream.
 */
function islandora_cwrc_writer_get_objects_with_cwrc_datastream() {
  $query = <<<EOT
PREFIX fedora-view: <info:fedora/fedora-system:def/view#>
SELECT ?pid
WHERE {
  ?disseminationType <fedora-view:disseminationType> <info:fedora/*/CWRC> .
  ?pid <fedora-view:disseminates> ?disseminationType
}
EOT;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->sparqlQuery($query);
  $documents = array();
  foreach ($results as $result) {
    $documents[] = $result['pid']['value'];
  }
  return $documents;
}

/**
 * Fetches a absolute url to the given objects RNG schema file.
 *
 * @param AbstractObject $object
 *   The document object.
 *
 * @return string
 *   The absolute url to the RNG schema file for validating the given document
 *   if the schema is found otherwise FALSE.
 */
function islandora_cwrc_writer_get_schema_url(AbstractObject $object) {
  $schema = islandora_cwrc_writer_get_document_schema_object($object);
  if ($schema) {
    return islandora_cwrc_writer_get_absolute_url("islandora/object/{$schema->id}/datastream/SCHEMA/view");
  }
  return FALSE;
}

/**
 * Gets the Schema PID from the given object if defined.
 *
 * @param AbstractObject $object
 *   The object to check.
 *
 * @return string|bool
 *   The PID if the given object has a schema, FALSE otherwise.
 */
function islandora_cwrc_writer_get_document_schema_pid(AbstractObject $object) {
  // Check if the object has a 'hasSchema' relationship.
  $relationship = $object->relationships->get(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_RELATIONSHIP);
  if ($relationship) {
    return $relationship[0]['object']['value'];
  }
  // Backwards compatiablity.
  $relationship = $object->relationships->get(ISLANDORA_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_RELATIONSHIP);
  if ($relationship) {
    return $relationship[0]['object']['value'];
  }
  return FALSE;
}

/**
 * Gets the schema object for the given document.
 *
 * If one is not defined on the given object, it's parent is checked. If it's
 * parent also does not specify a schema a default schema object will be
 * returned if it is found.
 *
 * @param AbstractObject $object
 *   The islandora object to add the schema relation to.
 *
 * @return AbstractObject
 *   The schema for the given document if found otherwise FALSE.
 */
function islandora_cwrc_writer_get_document_schema_object(AbstractObject $object) {
  $pid = islandora_cwrc_writer_get_document_schema_pid($object);
  if ($pid) {
    return islandora_object_load($pid);
  }
  else {
    // The document might be single page part of a collection as opposed to a
    // complete document onto itself.
    $relationship = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
    if ($relationship) {
      $parent_pid = $relationship[0]['object']['value'];
      $parent = islandora_object_load($parent_pid);
      if ($parent) {
        $pid = islandora_cwrc_writer_get_document_schema_pid($parent);
        if ($pid) {
          return islandora_object_load($pid);
        }
      }
    }
  }
  return islandora_cwrc_writer_get_document_default_schema_object();
}

/**
 * Returns a list of valid schema mappings.
 *
 * @return array
 *   The valid mappings for schema.
 */
function islandora_cwrc_writer_get_valid_schema_mappings() {
  return array(
    ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_TEI,
    ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
    ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ENTRY,
  );
}

/**
 * Returns a list of valid schema mappings.
 *
 * @return array
 *   The valid mappings for schema.
 */
function islandora_cwrc_writer_is_valid_schema_mapping($mapping) {
  $valid_mappings = islandora_cwrc_writer_get_valid_schema_mappings();
  return in_array($mapping, $valid_mappings);
}

/**
 * Gets the mapping from the given schema object if defined.
 *
 * The mapping is used to determine what elements and dialog boxes the
 * CWRC-Writer will use.
 *
 * @param AbstractObject $schema
 *   The schema object to check.
 *
 * @return string|bool
 *   The mapping if the given schema defines one, FALSE otherwise.
 *   The valid values for a mapping are: 'tei', 'orlando', 'cwrcEntry'.
 */
function islandora_cwrc_writer_get_schema_mapping(AbstractObject $schema) {
  // Check if the object has a 'hasSchemaMapping' relationship.
  $relationship = $schema->relationships->get(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP);
  if ($relationship) {
    return $relationship[0]['object']['value'];
  }
  return FALSE;
}

/**
 * Gets the template mapping from the given schema object if defined.
 *
 * The mapping is used to determine what elements and dialog boxes the
 * CWRC-Writer will use.
 *
 * @param AbstractObject $schema
 *   The schema object to check.
 *
 * @return string|bool
 *   The mapping if the given schema defines one, FALSE otherwise.
 *   The valid values for a mapping are: 'tei', 'orlando', 'cwrcEntry'.
 */
function islandora_cwrc_writer_get_template_mapping(AbstractObject $schema) {
  $note_rel = $schema->relationships->get(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE);
  $cite_rel = $schema->relationships->get(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE);
  if ($note_rel && $cite_rel) {
    return array(
      ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => $note_rel[0]['object']['value'],
      ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => $cite_rel[0]['object']['value']
    );
  }
  return FALSE;
}

/**
 * Sets the given Schema objects mapping.
 *
 * @param AbstractObject $schema
 *   The schema object to whose mapping will be set.
 *
 * @param string $mapping
 *   The mapping to use, expected to be valid mapping.
 */
function islandora_cwrc_writer_set_schema_mapping(AbstractObject $schema, $mapping) {
  if (islandora_cwrc_writer_is_valid_schema_mapping($mapping)) {
    $relationships = $schema->relationships;
    $relationships->autoCommit = FALSE;
    if (islandora_cwrc_writer_get_schema_mapping($schema) !== FALSE) {
      $relationships->remove(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP);
    }
    $relationships->registerNamespace('cwrc', CWRC_RELS_EXT_URI);
    $relationships->add(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP, $mapping, RELS_TYPE_PLAIN_LITERAL);
    $relationships->commitRelationships();
  }
}


/**
 * Sets the given Schema objects template mapping.
 *
 * @param AbstractObject $schema
 *   The schema object to whose mapping will be set.
 *
 * @param array $mapping
 *   The mapping to use, expected to be valid mapping.
 */
function islandora_cwrc_writer_set_template_mapping(AbstractObject $schema, $mapping) {
  if (islandora_cwrc_writer_is_valid_schema_mapping($mapping[ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP])) {
    $relationships = $schema->relationships;
    $relationships->autoCommit = FALSE;
    if (islandora_cwrc_writer_get_schema_mapping($schema) !== FALSE) {
      $relationships->remove(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE);
      $relationships->remove(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE);
    }
    $relationships->registerNamespace('cwrc', CWRC_RELS_EXT_URI);
    $relationships->add(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE, $mapping[ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE], RELS_TYPE_PLAIN_LITERAL);
    $relationships->add(CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE, $mapping[ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE], RELS_TYPE_PLAIN_LITERAL);
    $relationships->commitRelationships();
  }
}

/**
 * Guesses the mapping from the given schema file.
 *
 * We can't know for sure what schema we're dealing with so we guess based on
 * the namespace or root element and hope for the best.
 *
 * The mapping is used to determine what elements and dialog boxes the
 * CWRC-Writer will use. It is a requirement for the CWRC-Writer to function.
 *
 * @param string $file
 *   The schema file to check.
 *
 * @return string|bool
 *   The mapping if we can guess it, FALSE otherwise. The valid values for a
 *   mapping are: 'tei', 'orlando', 'cwrcEntry'.
 */
function islandora_cwrc_writer_guess_schema_mapping_from_schema($file) {
  $doc = new DOMDocument();
  $loaded = @$doc->load($file);
  if (!$loaded) {
    return FALSE;
  }
  // TEI RNG schemas are expected to declare their namespace.
  if ($doc->documentElement->hasAttribute('ns')) {
    $namespace = $doc->documentElement->getAttribute('ns');
    if ($namespace === 'http://www.tei-c.org/ns/1.0') {
      return array(
        ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_TEI,
        ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEI,
        ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEI 
      );
    }
  }

  // Orlando.
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('rng', 'http://relaxng.org/ns/structure/1.0');
  // Orlando Biography
  $results = $xpath->query('//rng:start/rng:element[@name="BIOGRAPHY"]');
  if ($results->length !== 0) {
    return array(
      ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_ORLANDO_BIOGRAPHY,
      ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => ''
    );
  }
  // Orlando Writing
  $results = $xpath->query('//rng:start/rng:element[@name="WRITING"]');
  if ($results->length !== 0) {
    return array(
      ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_ORLANDO_WRITING,
      ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => ''
    );
  }
  // Orlando Events  
  $results = $xpath->query('//rng:start/rng:element[@name="EVENTS" or @name="EVENT"]');
  if ($results->length !== 0) {
    return array(
      ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ORLANDO,
      ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_ORLANDO_EVENT,
      ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => ''
    );
  }

  // CWRC Entry.
  $results = $xpath->query('//rng:start/rng:choice/rng:element[@name="CWRC"]');
  if ($results->length !== 0) {
    return array(
      ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_RELATIONSHIP => ISLANDORA_CWRC_WRITER_SCHEMA_MAPPING_ENTRY,
      ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_TEMPLATE => ISLANDORA_CWRC_WRITER_SCHEMA_NOTE_ENTRY,
      ISLANDORA_CWRC_WRITER_SCHEMA_CITATION_TEMPLATE => ''
    );
  }
  return FALSE;
}

/**
 * Gets the default schema object for validating documents.
 *
 * @return AbstractObject
 *   The default schema object if it exists, false otherwise.
 */
function islandora_cwrc_writer_get_document_default_schema_object() {
  $pid = variable_get('islandora_cwrc_writer_default_schema_object', 'cwrc:teiSchema');
  return islandora_object_load($pid);
}

/**
 * Sets the schema to use for the given document object.
 *
 * @param AbstractObject $object
 *   The document object.
 * @param AbstractObject $schema
 *   The schema to use with the given document object.
 */
function islandora_cwrc_writer_set_document_schema(AbstractObject $object, AbstractObject $schema) {
  // We prefer documents split across pages falling back to the current object.
  $rels = $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf');
  $parent_rels = reset($rels);
  $object_to_relate = $parent_rels ? islandora_object_load($parent_rels['object']['value']) : $object;
  $relationships = $object_to_relate->relationships;
  // Replace relationship to the given schema.
  $relationships->autoCommit = FALSE;
  $relationships->remove(
    CWRC_RELS_EXT_URI,
    ISLANDORA_CWRC_WRITER_SCHEMA_RELATIONSHIP
  );
  // Backwards compatiablity.
  $relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_CWRC_WRITER_SCHEMA_RELATIONSHIP
  );
  $relationships->registerNamespace('cwrc', CWRC_RELS_EXT_URI);
  $relationships->add(
    CWRC_RELS_EXT_URI, ISLANDORA_CWRC_WRITER_SCHEMA_RELATIONSHIP,
    $schema->id
  );
  $relationships->commitRelationships();
}

/**
 * Adds the given processing instruction to the given DOMDocument.
 *
 * @param DOMDocument $doc
 *   The XML document to add the processing instructions to.
 * @param string $instruction
 *   The instruction text.
 * @param string $type
 *   The type of processing instruction to add.
 */
function islandora_cwrc_writer_add_processing_instruction(DOMDocument $doc, $instruction, $type) {
  // Remove any existing processing instructions of this type.
  $doc = islandora_cwrc_writer_remove_processing_instructions($doc, $type);
  $processing_instruction = $doc->createProcessingInstruction($type, $instruction);
  $doc->insertBefore($processing_instruction, $doc->documentElement);
  return $doc;
}

/**
 * Removes a type of processing instruction from the given DOMDocument.
 *
 * @param DOMDocument $doc
 *   The document to remove the processing instruction from.
 * @param string $type
 *   The type of processing instruction to remove.
 *
 * @return DOMDocument
 *   The modified XML.
 */
function islandora_cwrc_writer_remove_processing_instructions(DOMDocument $doc, $type) {
  if ($doc->hasChildNodes()) {
    foreach ($doc->childNodes as $child_node) {
      if (is_a($child_node, 'DOMProcessingInstruction') && $child_node->nodeName == $type) {
        $child_node->parentNode->removeChild($child_node);
      }
    }
  }
  return $doc;
}

/**
 * Adds the given stylesheet processing instructions for the given DOMDocument.
 *
 * @param DOMDocument $doc
 *   The XML document to add the xml-stylesheet processing instructions to.
 * @param string $url
 *   An absolute URL to a stylesheet.
 * @param string $mimetype
 *   The stylesheet mimetype.
 *
 * @return DOMDocument
 *   The modified XML.
 */
function islandora_cwrc_writer_add_stylesheet_processing_instruction(DOMDocument $doc, $url, $mimetype) {
  // Double quotes are required or else the CWRC-Writer will break trying to
  // parse it because it uses a poor regular expression.
  $instructions = 'href="' . $url . '" type="' . $mimetype . '"';
  return islandora_cwrc_writer_add_processing_instruction($doc, $instructions, 'xml-stylesheet');
}

/**
 * Adds the given xml-stylesheet processing instructions for a given string.
 *
 * @param string $xml
 *   The XML document to add the stylesheet processing instructions to.
 * @param string $url
 *   An absolute URL to a stylesheet.
 * @param string $mimetype
 *   The stylesheet mimetype.
 *
 * @return string
 *   The XML document with the processing instruction added if successful;
 *   FALSE otherwise.
 */
function islandora_cwrc_writer_add_stylesheet_processing_instruction_to_string($xml, $url, $mimetype) {
  $doc = islandora_cwrc_writer_load_document_from_string($xml);
  if ($doc) {
    $doc = islandora_cwrc_writer_add_stylesheet_processing_instruction($doc, $url, $mimetype);
    return $doc->saveXML();
  }
  return FALSE;
}

/**
 * Adds the given xml-model processing instructions for the given DOMDocument.
 *
 * @param DOMDocument $doc
 *   The XML document to add the xml-model processing instructions to.
 * @param string $url
 *   An absolute URL to a RNG-XML Schema.
 *
 * @return DOMDocument
 *   The XML document with the processing instruction added to it.
 */
function islandora_cwrc_writer_add_model_processing_instruction(DOMDocument $doc, $url) {
  $instructions = 'href="' . $url . '" type="application/xml" schematypens="http://relaxng.org/ns/structure/1.0"';
  return islandora_cwrc_writer_add_processing_instruction($doc, $instructions, 'xml-model');
}

/**
 * Adds the given xml-model processing instructions for the given XML file.
 *
 * @param string $filename
 *   The path to the file to add the processing instruction to.
 * @param string $url
 *   An absolute URL to a RNG-XML Schema.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_cwrc_writer_add_model_processing_instruction_to_file($filename, $url) {
  $doc = islandora_cwrc_writer_load_document_from_file($filename);
  if ($doc) {
    $doc = islandora_cwrc_writer_add_model_processing_instruction($doc, $url);
    return ($doc->save($filename) !== FALSE);
  }
  return FALSE;
}

/**
 * Adds the given xml-model processing instructions for the given XML file.
 *
 * @param string $xml
 *   The XML document to add the model processing instruction to.
 * @param string $url
 *   An absolute URL to a RNG-XML Schema.
 *
 * @return string
 *   The XML document with the processing instruction added if successful, FALSE
 *   otherwise.
 */
function islandora_cwrc_writer_add_model_processing_instruction_to_string($xml, $url) {
  $doc = islandora_cwrc_writer_load_document_from_string($xml);
  if ($doc) {
    $doc = islandora_cwrc_writer_add_model_processing_instruction($doc, $url);
    return $doc->saveXML();
  }
  return FALSE;
}

/**
 * Remove the xml-model processing instructions from the given DOMDocument.
 *
 * @param DOMDocument $doc
 *   The XML document to remove the xml-model processing instructions from.
 *
 * @return DOMDocument
 *   The XML document with the processing instruction removed.
 */
function islandora_cwrc_writer_remove_model_processing_instructions(DOMDocument $doc) {
  return islandora_cwrc_writer_remove_processing_instructions($doc, 'xml-model');
}

/**
 * Removes the xml-model processing instructions from the given XML file.
 *
 * @param string $filename
 *   The XML document to remove the model processing instruction from.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_cwrc_writer_remove_model_processing_instruction_from_file($filename) {
  $doc = islandora_cwrc_writer_load_document_from_file($filename);
  if ($doc) {
    $doc = islandora_cwrc_writer_remove_model_processing_instructions($doc);
    return ($doc->save($filename) !== FALSE);
  }
  return FALSE;
}

/**
 * Removes the xml-model processing instructions form the given XML file.
 *
 * @param string $xml
 *   The XML document to remove the model processing instruction from.
 *
 * @return string
 *   The XML document with the processing instruction removed if successful,
 *   FALSE otherwise.
 */
function islandora_cwrc_writer_remove_model_processing_instruction_from_string($xml) {
  $doc = islandora_cwrc_writer_load_document_from_string($xml);
  if ($doc) {
    $doc = islandora_cwrc_writer_remove_model_processing_instructions($doc);
    return $doc->saveXML();
  }
  return FALSE;
}

/**
 * Loads a document from the given file.
 *
 * Respects the required DOMDocument options for a document to work in the
 * CWRC-Writer.
 *
 * @param string $filename
 *   The path to the XML document to load.
 *
 * @return bool|DOMDocument
 *   The XML document if successfully loaded, FALSE otherwise.
 */
function islandora_cwrc_writer_load_document_from_file($filename) {
  $doc = islandora_cwrc_writer_create_domdocument();
  // If a processing instruction is present it will throw a warning so we
  // silence it.
  $filename = drupal_realpath($filename);
  if (@$doc->load($filename)) {
    return $doc;
  }
  return FALSE;
}

/**
 * Loads a document from the given string.
 *
 * Respects the required DOMDocument options for a document to work in the
 * CWRC-Writer.
 *
 * @param string $xml
 *   The XML document to load.
 *
 * @return bool|DOMDocument
 *   The XML document if successfully loaded, FALSE otherwise.
 */
function islandora_cwrc_writer_load_document_from_string($xml) {
  $doc = islandora_cwrc_writer_create_domdocument();
  // If a processing instruction is present it will throw a warning so we
  // silence it.
  if (@$doc->loadXML($xml)) {
    return $doc;
  }
  return FALSE;
}

/**
 * Creates a DOMDocument with required options for use in the CWRC-Writer.
 *
 * @return DOMDocument
 *   A DOMDocument with required options for use in the CWRC-Writer
 */
function islandora_cwrc_writer_create_domdocument() {
  $doc = new DOMDocument("1.0", "UTF-8");
  // We must preserve whitespace and not format the output for the
  // EMIC / CWRC-Writer newlines to <br> tags feature.
  $doc->preserveWhiteSpace = TRUE;
  $doc->formatOutput = FALSE;
  return $doc;
}

/**
 * Gets the list of all the schema's in fedora.
 *
 * @return array
 *   An array of associative arrays where each represents a single schema.
 *   - pid: The PID of the schema object.
 *   - name: The label / name of the schema.
 *   - url: An absolute URL to the RNG schema file.
 *   - cssUrl: An absolute URL to the CSS file.
 *   - schemaMappingsId: Indicates what mapping to use in the CWRC-Writer.
 *   - aliases: An array containing the non-URL-encoded URL to the RNG.
 */
function islandora_cwrc_writer_get_schemas() {
  global $base_url;
  $content_model = ISLANDORA_CWRC_WRITER_SCHEMA_CONTENT_MODEL;
  $cwrc_namespace = CWRC_RELS_EXT_URI;
  $query = <<<EOT
  PREFIX cwrc: <$cwrc_namespace>
  PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
  PREFIX fedora-view: <info:fedora/fedora-system:def/model#>
  SELECT ?pid ?label ?mapping ?entityTemplateNote ?entityTemplateCitation
  WHERE {
    ?pid <fedora-model:hasModel> <info:fedora/$content_model> .
    ?pid <fedora-model:label> ?label .
    ?pid <cwrc:hasSchemaMapping> ?mapping .
    ?pid <cwrc:hasEntityTemplateNote> ?entityTemplateNote .
    ?pid <cwrc:hasEntityTemplateCitation> ?entityTemplateCitation .
  }
EOT;
  $connection = islandora_get_tuque_connection();
  $results = $connection->repository->ri->sparqlQuery($query);
  $schemas = array();
  foreach ($results as $result) {
    $schema_object = islandora_object_load($result['pid']['value']);
    if ($schema_object) {
      // We shouldn't add schemas without CSS or SCHEMA datastreams; this could
      // confuse CWRC-Writer.
      if (!isset($schema_object['SCHEMA']) || !isset($schema_object['CSS'])) {
        continue;
      }
      // template info.
      $cwrc_writer_path = libraries_get_path(ISLANDORA_CWRC_WRITER_LIB);
      $entityTemplates = array(
        'note' => file_create_url($cwrc_writer_path."/src/".$result['entityTemplateNote']['value']),
        'citation' => file_create_url($cwrc_writer_path."/src/".$result['entityTemplateCitation']['value']),

      );
      // Base info.
      $schema = array(
        'pid' => $schema_object->id,
        'name' => $result['label']['value'],
        'schemaMappingsId' => $result['mapping']['value'],
        'entityTemplates' => $entityTemplates,
      );
      // Determine where to set the RNG URL from.
      if ($schema_object['SCHEMA']->controlGroup == 'X' || $schema_object['SCHEMA']->controlGroup == 'M') {
        $schema['url'] = islandora_cwrc_writer_get_absolute_url("islandora/object/{$schema_object->id}/datastream/SCHEMA/view");
        $schema['aliases'] = array(
          // XXX: Generate an alias to match the old concatenated structure
          // (without URL-encoding).
          "$base_url/islandora/object/{$schema_object->id}/datastream/SCHEMA/view",
        );
      }
      else {
        $schema['url'] = $schema_object['SCHEMA']->url;
        $schema['aliases'] = array($schema_object['SCHEMA']->url);
      }
      // Do the same for the CSS URL.
      $schema['cssUrl'] = ($schema_object['CSS']->controlGroup == 'X' || $schema_object['CSS']->controlGroup == 'M') ?
        islandora_cwrc_writer_get_absolute_url("islandora/object/{$schema_object->id}/datastream/CSS/view") :
        $schema_object['CSS']->url;
      // Add it to the list.
      $schemas[$schema_object->id] = $schema;
    }
  }
  return $schemas;
}

/**
 * Gets an associative array that describes all the valid entity types.
 *
 * @return array
 *   An associative array where the keys represent the type, and the value
 *   represents the content model PID.
 */
function islandora_cwrc_writer_valid_entity_types() {
  return array(
    'person' => ISLANDORA_CWRC_WRITER_PERSON_ENTITY_CONTENT_MODEL,
    'organization' => ISLANDORA_CWRC_WRITER_ORGANIZATION_ENTITY_CONTENT_MODEL,
    'title' => ISLANDORA_CWRC_WRITER_TITLE_ENTITY_CONTENT_MODEL,
    'place' => ISLANDORA_CWRC_WRITER_PLACE_ENTITY_CONTENT_MODEL,
  );
}

/**
 * Gets an associative array that describes all the valid entity types.
 *
 * @return array
 *   An associative array where the keys represent the type, and the value
 *   represents the content model PID.
 */
function islandora_cwrc_writer_entity_content_datastreams() {
  return array(
    ISLANDORA_CWRC_WRITER_PERSON_ENTITY_CONTENT_MODEL => 'PERSON',
    ISLANDORA_CWRC_WRITER_ORGANIZATION_ENTITY_CONTENT_MODEL => 'ORGANIZATION',
    ISLANDORA_CWRC_WRITER_TITLE_ENTITY_CONTENT_MODEL => 'TITLE',
    ISLANDORA_CWRC_WRITER_PLACE_ENTITY_CONTENT_MODEL => 'PLACE',
  );
}

/**
 * Gets the entity type of the given object if applicable.
 *
 * @param AbstractObject $object
 *   The entity object in which to fetch the type of.
 *
 * @return string
 *   The entity type of the given object if applicable. FALSE if it's not
 *   applicable.
 */
function islandora_cwrc_writer_get_entity_type(AbstractObject $object) {
  $valid_types = islandora_cwrc_writer_valid_entity_types();
  foreach ($object->models as $model) {
    $key = array_search($model, $valid_types);
    if ($key !== FALSE) {
      return $key;
    }
  }
  return FALSE;
}

/**
 * Gets the DSID of the given entities content datastream.
 *
 * @param AbstractObject $object
 *   The entity object in which to fetch the datastream.
 *
 * @return string
 *   Returns the DSID of the given entities content datastream or FALSE if not
 *   applicable.
 */
function islandora_cwrc_writer_entity_content_datastream_id(AbstractObject $object) {
  $datastreams = islandora_cwrc_writer_entity_content_datastreams($object);
  foreach ($object->models as $model) {
    if (isset($datastreams[$model])) {
      return $datastreams[$model];
    }
  }
  return FALSE;
}

/**
 * Checks if the given type is a valid entity type.
 *
 * @param string $type
 *   The type to check.
 *
 * @return bool
 *   TRUE if it's valid FALSE otherwise.
 */
function islandora_cwrc_writer_valid_entity_type($type) {
  $valid_types = islandora_cwrc_writer_valid_entity_types();
  $type = strtolower($type);
  return isset($valid_types[$type]);
}

/**
 * Gets the default entity collection for the given type.
 *
 * @param string $type
 *   The type of entity whose default collection should be returned.
 *
 * @return string
 *   The PID of the default collection of the given $type. FALSE if none is
 *   defined.
 */
function islandora_cwrc_writer_default_entity_collection($type) {
  $variables = array(
    'person' => array(
      'islandora_cwrc_writer_person_entity_collection',
      'cwrc:personEntityCollection',
    ),
    'organization' => array(
      'islandora_cwrc_writer_organization_entity_collection',
      'cwrc:organizationEntityCollection',
    ),
    'title' => array(
      'islandora_cwrc_writer_title_entity_collection',
      'cwrc:titleEntityCollection',
    ),
    'place' => array(
      'islandora_cwrc_writer_place_entity_collection',
      'cwrc:placeEntityCollection',
    ),
  );
  if (isset($variables[$type])) {
    list($variable, $default) = $variables[$type];
    return variable_get($variable, $default);
  }
  return FALSE;
}

/**
 * Helper function to generate absolute URLs without language prefixes.
 *
 * @param string $path
 *   The path, as accepted by url().
 * @param array $options_in
 *   Options, as accepted by url(). 'absolute' and 'language' will be
 *   overridden.
 *
 * @return string
 *   The absolute URL without language prefixes.
 */
function islandora_cwrc_writer_get_absolute_url($path, $options_in = array()) {
  static $no_language = NULL;
  if ($no_language === NULL) {
    $no_language = new stdClass();
    $no_language->language = LANGUAGE_NONE;
  }

  return url($path, array(
    'absolute' => TRUE,
    'language' => $no_language,
  ) + $options_in);
}
