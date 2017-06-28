<?php

/**
 * @file
 * Functions for user interface access rights.
 */

function ubl_access_rights_from_metadata_accessible_dsids($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('allow_access_to_dsid','deny_access_to_dsid');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $accessibledsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of allowed dsids if it is in the deny dsids also.
  $accessibledsids = array_diff($accessibledsids, $denydsids);

  if ($realobjid !== NULL) {
    if ($obj->id !== $accesssettings['source_of_settings']) {
      $realobjid = $accesssettings['source_of_settings'];
    }
  }

  return $accessibledsids;
}

function ubl_access_rights_from_metadata_restricted_dsids($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('allow_access_to_dsid','deny_access_to_dsid');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $accessibledsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of deny dsids if it is in the allowed dsids also.
  $restricteddsids = array_diff($denydsids, $accessibledsids);

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['source_of_settings']) {
      $realobjid = $accesssettings['source_of_settings'];
    }
  }

  return $restricteddsids;
}

function ubl_access_rights_from_metadata_allowed_roles($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('allow_access_for_role');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $allowedroles = isset($accesssettings['allow_access_for_role'])?$accesssettings['allow_access_for_role']:array();

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['source_of_settings']) {
      $realobjid = $accesssettings['source_of_settings'];
    }
  }

  return $allowedroles;
}

function ubl_access_rights_from_metadata_downloadable_dsids_with_label($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('provide_download_of_dsid');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $downloadabledsids = _ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'provide_download_of_dsid', $obj, array('RELS-INT','RELS-EXT','POLICY'));

  $result = array();
  $dsid2label = isset($config['_config']['general']['dsid2label'])?$config['_config']['general']['dsid2label']:array();
  foreach ($downloadabledsids as $downloadabledsid) {
    $label = NULL;
    if (isset($dsid2label[$downloadabledsid])) {
      $label = $dsid2label[$downloadabledsid];
      if (isset($obj[$downloadabledsid])) {
        $label = str_replace("{dsid}", $downloadabledsid, $label);
        $label = str_replace("{mime}", $obj[$downloadabledsid]->mimeType, $label);
        $label = str_replace("{label}", $obj[$downloadabledsid]->label, $label);
      }
    }
    else if (isset($obj[$downloadabledsid])) {
      $label = $obj[$downloadabledsid]->label;
    }
    if (isset($label)) {
      $result[$downloadabledsid] = $label;
    }
  }

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['source_of_settings']) {
      $realobjid = $accesssettings['source_of_settings'];
    }
  }

  return $result;
}

function ubl_access_rights_from_metadata_access_description($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('access_text','access_usetext','access_link','access_image');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $result = array();
  $result['accesstext'] = isset($accesssettings['access_text'])?$accesssettings['access_text']:'';
  $result['accessusetext'] = isset($accesssettings['access_usetext'])?$accesssettings['access_usetext']:'';
  $result['accesslink'] = isset($accesssettings['access_link'])?$accesssettings['access_link']:'';
  $result['accessimage'] = isset($accesssettings['access_image'])?$accesssettings['access_image']:'';

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['source_of_settings']) {
      $realobjid = $accesssettings['source_of_settings'];
    }
  }

  return $result;
}

function ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

  $accessvalue = ubl_access_rights_from_metadata_get_access_rights_value($obj, $config);
  $accesssettings = ubl_access_rights_from_metadata_get_access_rights_settings($accessvalue, $config, FALSE);

  $part_of = NULL;
  if ($accesssettings === NULL) {
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
          $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($pobj, $config, $neededkeys);
          if ($accesssettings !== NULL) {
            $accesssettings['source_of_settings'] = $ppid;
            break; 
          }
        }
      }
    }
  }
  if (($accesssettings === NULL) || !ubl_access_rights_from_metadata_settings_has_any_needed_keys($accesssettings, $neededkeys)) {
    $accesssettings = ubl_access_rights_from_metadata_get_access_rights_settings($accessvalue, $config, TRUE, $neededkeys);
    if ($accesssettings) {
      $accesssettings['source_of_settings'] = 'otherwise';
    }
  }
  if (!isset($accesssettings['source_of_settings'])) {
    $accesssettings['source_of_settings'] = '';
  }

  return $accesssettings;
}
