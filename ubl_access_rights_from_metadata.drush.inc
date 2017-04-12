<?php
/**
 * @file
 * Functions for batch changing the XACML policy of objects.
 */

/**
 * Implements hook_drush_command().
 */
function ubl_access_rights_from_metadata_drush_command() {
  $items['change_xacml_policy_based_on_metadata'] = array(
    'description' => 'Change the XACML policy based on the metadata of the objects. An absolute path to a configuration directory with ini files or a single ini file should be supplied. Also a collection ID is mandatory.',
    'options' => array(
      'collection' => 'Mandatory, one collection ID.',
      'configuration' => 'Optional, the absolute path to either a directory containing ini files or the absolute filepath to an ini file. If omitted, uses piblic://ubl_access_rights_from_metadata. See documentation for specifics.',
    ),
    'aliases' => array('cxpbomd'),
    'examples' => array(
      'drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/path/to/directory/containing/ini/files',
      'drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/path/to/a/ini/file',
      'drush --user=admin cxpbomd --collection=islandora:root',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_access_rights_from_metadata_change_xacml_policy_based_on_metadata_validate() {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configuration = drush_get_option('configuration');
  $collection = drush_get_option('collection');
  $configdir = '';
  $configspecificfile = '';

  // check configuration option
  if (!isset($configuration)) {
    $configuration = drupal_realpath("public://ubl_access_rights_from_metadata");
  }
  if (is_dir($configuration)) {
    $configdir = rtrim($configuration, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
    $error = '';
    $valid = ubl_access_rights_from_metadata_valid_config_directory($configdir, $error);
    if (!$valid) {
      return drush_set_error($error);
    }
  }
  elseif (is_file($configuration)) {
    $error = '';
    if (!ubl_access_rights_from_metadata_valid_config_file($configuration, $error)) {
      return drush_set_error($error);
    }
    $pathinfo = pathinfo($configuration);
    $configdir = $pathinfo['dirname'] . DIRECTORY_SEPARATOR;
    $configspecificfile = $pathinfo['basename'];
  }
  else {
    return drush_set_error("configuration '$configuration' does not exist, should be an absolute path to an existing directory or file.");
  }

  // check collection option
  if (!isset($collection)) {
    return drush_set_error("Argument collection is mandatory!");
  }
  $collectionobj = islandora_object_load($collection);
  if (!($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models))) {
    return drush_set_error("Collection '$collection' does not resolve to a collection");
  }
 
  return TRUE;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_ubl_access_rights_from_metadata_change_xacml_policy_based_on_metadata() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/xacml');
  module_load_include('inc', 'islandora_xacml_api', 'includes/islandora_xacml');

  $configuration = drush_get_option('configuration');
  $collection = drush_get_option('collection');
  $configdir = NULL;
  $configspecificfile = NULL;

  if (!isset($configuration)) {
    $configuration = drupal_realpath("public://ubl_access_rights_from_metadata");
  }
  if (is_dir($configuration)) {
    $configdir = rtrim($configuration, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
  }
  elseif (is_file($configuration)) {
    $pathinfo = pathinfo($configuration);
    $configdir = $pathinfo['dirname'] . DIRECTORY_SEPARATOR;
    $configspecificfile = $pathinfo['basename'];
  }

  $collectionobj = islandora_object_load($collection);
  if ($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models)) {
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($collectionobj, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $cobj = islandora_object_load($cid);
      if ($cobj) { 
        $options = array('object' => $cobj, 'directory' => $configdir);
        if (isset($configspecificfile)) {
          $options['file'] = $configspecificfile;
        }
        $config = ubl_access_rights_from_metadata_get_configuration($options);
        ubl_access_rights_from_metadata_xacml_modify_if_needed($cobj, $config); 
      }
    }
  }
}
