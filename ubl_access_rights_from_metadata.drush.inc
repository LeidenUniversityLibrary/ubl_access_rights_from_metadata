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
  $items['check_access_rights_settings'] = array(
    'description' => 'Check the access rights settings of the children of a specific collection.',
    'options' => array(
      'collection' => 'Mandatory, one collection ID.',
      'configuration' => 'Mandatory, the absolute filepath to an ini file.',
    ),
    'aliases' => array('cars'),
    'examples' => array(
      'drush --user=admin check_access_rights_settings --collection=islandora:root --configuration=/path/to/a/ini/file',
      'drush --user=admin check_access_rights_settings --collection=islandora:root --configuration=/url/to/a/ini/file',
      'drush --user=admin cars --collection=islandora:root --configuration=/url/to/a/ini/file',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_access_rights_from_metadata_change_xacml_policy_based_on_metadata_validate() {
  return _ubl_access_rights_from_metadata_validate_collection_configuration();
}

function _ubl_access_rights_from_metadata_validate_collection_configuration() {
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

  if (!variable_get('ubl_access_rights_from_metadata_enable_xacml', FALSE)) {
    if (!drush_confirm('XACML policy updates are disabled in admin/islandora/tools/ubl_access_rights_from_metadata. Are you sure you want to continue?')) {
      drush_log('Stopped');
      return;
    }
  }

  $configlocation = drush_get_option('configuration');
  $collection = drush_get_option('collection');

  $collectionobj = islandora_object_load($collection);
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  if ($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models)) {
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($collectionobj, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $cobj = islandora_object_load($cid);
      if ($cobj) { 
        ubl_access_rights_from_metadata_xacml_modify_if_needed($cobj, $config); 
      }
    }
  }
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_access_rights_from_metadata_check_access_rights_settings_validate() {
  return _ubl_access_rights_from_metadata_validate_collection_configuration();
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_ubl_access_rights_from_metadata_check_access_rights_settings() {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_settings');

  $configlocation = drush_get_option('configuration');
  $collection = drush_get_option('collection');

  $collectionobj = islandora_object_load($collection);
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  if ($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models)) {
    $accessvalue2objids = array();
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($collectionobj, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $cobj = islandora_object_load($cid);
      if ($cobj) {
        $ars = ubl_access_rights_from_metadata_access_rights_settings_for_object($cobj, $config); 
        $av = '';
        if (isset($ars['access_text'])) {
          $av = $ars['access_text'];
        }
        else {
          $av = '(access undefined)';
        }
        if (isset($ars['access_usetext'])) {
          $av = $av . ' -- ' . $ars['access_usetext'];
        }
        else {
          $av = $av . ' -- ' . '(use undefined)';
        }
        if (!isset($accessvalue2objids[$av])) {
          $accessvalue2objids[$av] = array();
        }
        $accessvalue2objids[$av][] = $cid;
      }
    }
    print "\nCheck access rights value results:\n";
    print "==================================\n";
    ksort($accessvalue2objids);
    foreach ($accessvalue2objids as $av => $objids) {
      printf("%5d x %s\n", count($objids), $av);
    }
    print "\n";
  }
}
