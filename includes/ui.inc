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

  $dsids = array();
  $specificaccesssettings = array();
  if (isset($accesssettings['all'])) {
    $specificaccesssettings = $accesssettings['all'];
  }
  else if (isset($accesssettings['external'])) {
    $specificaccesssettings = $accesssettings['external'];
  }
  else if (isset($accesssettings['internal'])) {
    $specificaccesssettings = $accesssettings['internal'];
  }
  if (isset($specificaccesssettings['provide_download_of_dsid'])) {
    $downloadabledsids = $specificaccesssettings['provide_download_of_dsid'];
    $downloadabledsids = _unfold_dsid_array($downloadabledsids, $obj, array('RELS-INT','RELS-EXT','POLICY'));

    foreach ($obj as $ds) {
      $dsid = $ds->id;
      if (in_array($dsid, $downloadabledsids)) {
        $dsids[] = $dsid;
      }
    }
  }
  return $dsids;
}

function ubl_access_rights_from_metadata_access_description($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $config = ubl_access_rights_from_metadata_get_configuration(array('object' => $obj));
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $result = array();
  $specificaccesssettings = array();
  if (isset($accesssettings['all'])) {
    $specificaccesssettings = $accesssettings['all'];
  }
  else if (isset($accesssettings['external'])) {
    $specificaccesssettings = $accesssettings['external'];
  }
  else if (isset($accesssettings['internal'])) {
    $specificaccesssettings = $accesssettings['internal'];
  }
  $result['accesstext'] = isset($specificaccesssettings['access_text'])?$specificaccesssettings['access_text']:'';
  $result['accesslink'] = isset($specificaccesssettings['access_link'])?$specificaccesssettings['access_link']:'';
  $result['accessimage'] = isset($specificaccesssettings['access_image'])?$specificaccesssettings['access_image']:'';

  return $result;
}

function ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $accessvalue = ubl_access_rights_from_metadata_get_access_rights_value($obj, $config);
  $accesssettings = ubl_access_rights_from_metadata_get_settings($accessvalue, $config);

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
