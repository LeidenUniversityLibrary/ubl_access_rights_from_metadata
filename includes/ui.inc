<?php

/**
 * @file
 * Functions for user interface access rights.
 */

function ubl_access_rights_from_metadata_downloadable_dsids($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $config = ubl_access_rights_from_metadata_get_configuration(array('object' => $obj));
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $downloadabledsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'provide_download_of_dsid', $obj, array('RELS-INT','RELS-EXT','POLICY'));

  return $downloadabledsids;
}

function ubl_access_rights_from_metadata_access_description($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $config = ubl_access_rights_from_metadata_get_configuration(array('object' => $obj));
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $result = array();
  $result['accesstext'] = isset($accesssettings['access_text'])?$accesssettings['access_text']:'';
  $result['accesslink'] = isset($accesssettings['access_link'])?$accesssettings['access_link']:'';
  $result['accessimage'] = isset($accesssettings['access_image'])?$accesssettings['access_image']:'';

  return $result;
}

function ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $accessvalue = ubl_access_rights_from_metadata_get_access_rights_value($obj, $config);
  $accesssettings = ubl_access_rights_from_metadata_get_access_rights_settings($accessvalue, $config);

  if ($accesssettings === NULL) {
    $part_of = NULL;
    // Book page?
    $part_of = $obj->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
    // Other member?
    if (empty($part_of)) {
      $part_of = $obj->relationships->get(ISLANDORA_RELS_EXT_URI, 'isMemberOf');
    }
    if (empty($part_of)) {
    // Compound part?
      $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
      $part_of = $obj->relationships->get(FEDORA_RELS_EXT_URI, $rels_predicate);
    }

    if (!empty($part_of) && is_array($part_of)) {
      foreach ($part_of as $part) {
        $ppid = $part['object']['value'];
        $pobj = islandora_object_load($ppid);
        if ($pobj) {
          $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($pobj, $config);
          if ($accesssettings !== NULL) {
            return $accesssettings; 
          }
        }
      }
    }
  }

  return $accesssettings;
}
