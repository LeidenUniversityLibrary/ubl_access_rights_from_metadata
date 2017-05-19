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
      'configuration' => 'Mandatory, the absolute filepath to an ini file.',
    ),
    'aliases' => array('cxpbomd'),
    'examples' => array(
      'drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/path/to/a/ini/file',
      'drush --user=admin change_xacml_policy_based_on_metadata --collection=islandora:root --configuration=/url/to/a/ini/file',
      'drush --user=admin cxpbomd --collection=islandora:root --configuration=/url/to/a/ini/file',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_access_rights_from_metadata_change_xacml_policy_based_on_metadata_validate() {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = drush_get_option('configuration');
  $collection = drush_get_option('collection');
  $configdir = '';
  $configspecificfile = '';

  // check configuration option
  if (!isset($configlocation)) {
    return drush_set_error("Argument configuration is mandatory!");
  }
  $error = '';
  if (!ubl_access_rights_from_metadata_valid_config_file($configlocation, $error)) {
    return drush_set_error($error);
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

  $configlocation = drush_get_option('configuration');
  $collection = drush_get_option('collection');

  $collectionobj = islandora_object_load($collection);
  if ($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models)) {
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($collectionobj, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $cobj = islandora_object_load($cid);
      if ($cobj) { 
        $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
        ubl_access_rights_from_metadata_xacml_modify_if_needed($cobj, $config); 
      }
    }
  }
}
