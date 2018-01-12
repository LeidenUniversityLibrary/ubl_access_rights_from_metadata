<?php

/**
 * @file
 * Calculate the access rights settings for a specific object.
 */

function ubl_access_rights_from_metadata_access_rights_settings_for_object($object, $config, $neededkeys = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/metadata_value');

  $accessrights = array();
  
  $conditionconfig = $config['condition'];
  $conditions = array();
  $reason = array();
  $conditionsmet = array();
  $isipdependent = FALSE;

  foreach ($conditionconfig as $conditionname => $conditionvalues) {
    $conditions[$conditionname] = _ubl_access_rights_from_metadata_evaluate_condition($conditionvalues, $object, $conditions, $config);
    if ($conditions[$conditionname]) {
      $conditionsmet[$conditionname] = FALSE;
    }
  }
  
  $accessconfig = $config['access'];
  foreach ($accessconfig as $accessname => $accessvalues) {
    if (isset($conditions[$accessname]) && $conditions[$accessname]) {
      foreach ($accessvalues as $key => $value) {
        if (!isset($accessrights[$key])) {
          $reason[$key][$accessname] = $object->id;
        }
      }
      $accessrights = $accessrights + $accessvalues;
      $conditionsmet[$accessname] = TRUE;
      if (!$isipdependent && _ubl_access_rights_from_metadata_is_ip_dependent($accessname, $config)) {
        $isipdependent = TRUE;
      }
    }
  }
  $accessrights['_conditions_met'] = $conditionsmet;
  $accessrights['_ip_dependent'] = $isipdependent;
  $accessrights['_reasons'] = $reason;

  return $accessrights;
}

function _ubl_access_rights_from_metadata_is_ip_dependent($conditionname, $config) {
  $isipdependent = FALSE;
  if (array_key_exists('ip_range', $config['condition'][$conditionname])) {
    $isipdependent = TRUE;
  }
  if (!$isipdependent && array_key_exists('condition', $config['condition'][$conditionname])) {
    foreach ($config['condition'][$conditionname]['condition'] as $subconditionname) {
      $isipdependent = $isipdependent || _ubl_access_rights_from_metadata_is_ip_dependent($subconditionname, $config);
    }
  }
  return $isipdependent;
}

function _ubl_access_rights_from_metadata_evaluate_condition($conditionvalues, $object, $existingconditions, $config) {
  $results = array();
  $operator = 'and';
  if (isset($conditionvalues['operator'])) {
    $operator = $conditionvalues['operator'];
  }
  if (isset($conditionvalues['metadata'], $conditionvalues['comparator'], $conditionvalues['value'])) {
    $mdvalue = ubl_access_rights_from_metadata_get_metadata_value($object, $config, $conditionvalues['metadata']); 
    $comparator = $conditionvalues['comparator'];
    $value = $conditionvalues['value'];
    $prefix = '_ubl_access_rights_from_metadata_comparator_';
    $compfunc = $prefix . $comparator;

    $results[] = $compfunc($mdvalue, $value);
  }
  if (isset($conditionvalues['user_role'])) {
    global $user;
    $results[] = (count(array_intersect($user->roles, $conditionvalues['user_role'])) > 0);
  }
  if (isset($conditionvalues['ip_range'])) {
    $ipranges = $conditionvalues['ip_range'];
    $isinrange = FALSE;
    $clientip = ip_address();
    $clientiphex = ipToHex($clientip);
    if ($clientiphex) {
      foreach ($ipranges as $iprange) {
        $min = $iprange['min'];
        $max = $iprange['max'];
        if (strcmp($min, $clientiphex) <= 0 && strcmp($clientiphex, $max) <= 0) {
          $isinrange = TRUE;
          continue;
        }
      }
    }
    $results[] = $isinrange; 
  }
  if (isset($conditionvalues['condition'])) {
    foreach ($conditionvalues['condition'] as $conditionname) {
      $results[] = $existingconditions[$conditionname];
    }
  }

  $result = FALSE;
  $trues = array_filter($results, function($v) { return ($v === TRUE); });
  if ($operator === 'or') {
    // OR
    $result = (count($trues) > 0);
  }
  else {
    // AND
    $result = (count($results) > 0) && (count($trues) === count($results));  
  }
  if ($operator === 'negate' || $operator === 'not') {
    // NOT
    $result = !$result;
  }
  return $result;
}

function _ubl_access_rights_from_metadata_comparator_lowercaseLetterMatch($value1, $value2) {
  $v1 = preg_replace('/[^a-z]/', '', strtolower($value1));
  $v2 = preg_replace('/[^a-z]/', '', strtolower($value2));

  return ($v1 === $v2);
}

function _ubl_access_rights_from_metadata_comparator_equals($value1, $value2) {
  return ($value1 === $value2);
}

function _ubl_access_rights_from_metadata_comparator_afterDateISO8601($value1, $value2) {
  $date1 = _ubl_access_rights_from_metadata_comparator_date_ISO8601_normalizer($value1);  
  $date2 = _ubl_access_rights_from_metadata_comparator_date_ISO8601_normalizer($value2);  

  if ($date1 === FALSE || $date2 === FALSE) {
    return FALSE;
  }

  return ($date1 > $date2);
}

function _ubl_access_rights_from_metadata_comparator_beforeDateISO8601($value1, $value2) {
  $date1 = _ubl_access_rights_from_metadata_comparator_date_ISO8601_normalizer($value1);  
  $date2 = _ubl_access_rights_from_metadata_comparator_date_ISO8601_normalizer($value2);  

  if ($date1 === FALSE || $date2 === FALSE) {
    return FALSE;
  }

  return ($date1 < $date2);
}

function _ubl_access_rights_from_metadata_comparator_date_ISO8601_normalizer($value) {
  $matches = array();
  if ($value === 'today') {
    return date("Y-m-d");
  }
  elseif ($value === 'now') {
    return date("Y-m-d\TH:i:s\Z");
  }
  elseif (preg_match('/^(\d{4})-?(\d\d)-?(\d\d)$/', $value, $matches) === 1) {
    return $matches[1] . '-' . $matches[2] . '-' . $matches[3]; 
  }
  elseif (preg_match('/^(\d{4})-?(\d\d)-?(\d\d)T(\d\d):?(\d\d):?(\d\d)Z?$/', $value, $matches) === 1) {
    return $matches[1] . '-' . $matches[2] . '-' . $matches[3] . 'T' . $matches[4] . ':' . $matches[5] . ':' . $matches[6] . 'Z'; 
  }
  elseif (preg_match('/^(\d{4})-?(\d\d)-?(\d\d)T(\d\d):?(\d\d):?(\d\d)([+-])(\d\d)(?::?(\d\d))?$/', $value, $matches) === 1) {
    $day = $matches[3];
    $hour = $matches[4];
    $minute = $matches[5];
    $factor = ($matches[7] === '-')?1:-1; // T22:30+04:00 => T18:30Z
    $offsethour = $matches[8];
    $offsetmin = $matches[9];
    $minute += $factor * $offsetmin;
    if ($minute < 0) {
      $minute += 60;
      $hour -= 1;
    }
    if ($minute > 59) {
      $minute -= 60;
      $hour += 1;
    }
    $hour += $factor * $offsethour;
    if ($hour < 0) {
      $hour += 24;
      $day -= 1;
    }
    if ($hour > 23) {
      $hour -= 24;
      $day += 1;
    }
    // there might be some problems with specific dates (e.g. 2018-01-01T01:00:00+04:00 becomes 2018-01-00T21:00Z),
    // but this isn't that big of a problem when comparing dates
    return $matches[1] . '-' . $matches[2] . '-' . $day . 'T' . $hour . ':' . $minute . ':' . $matches[6] . 'Z'; 
  }
  else {
    return FALSE;
  }
}

function ubl_access_rights_from_metadata_unfold_dsid_array($cnfg, $key, $obj, $dontmessdsids = array()) {
  if (!isset($cnfg[$key])) {
    return array();
  }       
  $dsids = $cnfg[$key];
  if (in_array('NONE', $dsids)) {
    return array();
  }     
  if (in_array('ALL', $dsids)) {
    $dsids = array();
    foreach ($obj as $ds) {
      $dsid = $ds->id;
      if (!in_array($dsid, $dontmessdsids)) {
        $dsids[] = $dsid;
      }
    }   
  }     
  return $dsids;
}     

function ubl_access_rights_from_metadata_accessible_object($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  if (isset($accesssettings['deny_viewing'])) {
    return !$accesssettings['deny_viewing'];
  }
  return NULL;
}

function ubl_access_rights_from_metadata_accessible_dsids($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $accessibledsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of allowed dsids if it is in the deny dsids also.
  $accessibledsids = array_diff($accessibledsids, $denydsids);

  return $accessibledsids;
}

function ubl_access_rights_from_metadata_restricted_dsids($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $accessibledsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of deny dsids if it is in the allowed dsids also.
  $restricteddsids = array_diff($denydsids, $accessibledsids);

  return $restricteddsids;
}

function ubl_access_rights_from_metadata_downloadable_dsids_with_label($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $downloadabledsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'provide_download_of_dsid', $obj, array('RELS-INT','RELS-EXT','POLICY'));

  $result = array();
  $dsid2label = ubl_access_rights_from_metadata_dsids_with_labels();
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

  return $result;
}

function ubl_access_rights_from_metadata_dsids_with_labels() {
  return array(
    "MODS"          => t("Descriptive Metadata (MODS)"),
    "DC"            => t("Descriptive Metadata (DC)"),
    "OBJ"           => t("Original master"),
    "JPG"           => t("Image (medium resolution)"),
    "JP2"           => t("Image (high resolution)"),
    "MEDIUM_SIZE"   => t("Image (medium resolution)"),
    "OCR"           => t("Text (OCR)"),
    "HOCR"          => t("Text (OCR XML)"),
    "PDF"           => t("PDF"),
    "PDFA"          => t("PDF/A"),
    "FULL_TEXT"     => t("Text (from PDF)"),
    "WAV"           => t("Audio (WAV)"),
    "Proxy_MP3"     => t("Audio (MP3)"),
    "PROXY_MP3"     => t("Audio (MP3)"),
    "OBJ_EXIFTOOL"  => t("Technical metadata"),
    "MP4"           => t("Video (MP4)"),
    "MKV"           => t("Video (MKV)"),
    "TECHMD"        => t("Technical metadata"),
    "TechMD_DFXML"  => t("Technical metadata"),
    "TECHMD_FITS"   => t("Technical metadata"),
    "EAD"           => t("Collection Guide (EAD XML)"),
    "WARC_CSV"      => t("Web archive"),
    "WARC_FILTERED" => t("Web archive"),
    "TN"            => t("Thumbnail"),
  );
}

function ubl_access_rights_from_metadata_access_description($obj) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config);

  $result = array();
  $result['accesstext'] = isset($accesssettings['access_text'])?$accesssettings['access_text']:'';
  $result['accessusetext'] = isset($accesssettings['access_usetext'])?$accesssettings['access_usetext']:'';
  $result['accesslink'] = isset($accesssettings['access_link'])?$accesssettings['access_link']:'';
  $result['accessimage'] = isset($accesssettings['access_image'])?$accesssettings['access_image']:'';

  return $result;
}

function ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config) {

  $settings = &drupal_static(__FUNCTION__);

  if (!isset($settings[$obj->id])) {
    module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  
    $accesssettings = ubl_access_rights_from_metadata_access_rights_settings_for_object($obj, $config);
    if (!empty($accesssettings)) {
      $accesssettings['_source_of_settings'][] = $obj->id;
    }
    $neededkeys = array_keys(ubl_access_rights_from_metadata_default_settings());
  
    $part_of = NULL;

    if (!ubl_access_rights_from_metadata_settings_has_all_needed_keys($accesssettings, $neededkeys)) {
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
            $paccesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($pobj, $config);
            if (!empty($paccesssettings)) {
              $accesssettings['_source_of_settings'] = array_unique(array_merge($accesssettings['_source_of_settings'], $paccesssettings['_source_of_settings']));
              foreach ($paccesssettings['_conditions_met'] as $pcond => $isaccesscond) {
                $accesssettings['_conditions_met'][$pcond] = (isset($accesssettings['_conditions_met'][$pcond]) && $accesssettings['_conditions_met'][$pcond]) || $isaccesscond;
              }
              $paccesssettings['_reasons'] = $accesssettings['_reasons'] + $paccesssettings['_reasons'];
              $accesssettings['_ip_dependent'] = $accesssettings['_ip_dependent'] || $paccesssettings['_ip_dependent'];
              $accesssettings = $accesssettings + $paccesssettings;
              if (ubl_access_rights_from_metadata_settings_has_all_needed_keys($accesssettings, $neededkeys)) {
                break; 
              }
            }
          }
        }
      }
    }
    if (!ubl_access_rights_from_metadata_settings_has_all_needed_keys($accesssettings, $neededkeys)) {
      $defaults = ubl_access_rights_from_metadata_default_settings();
      $didusedefault = FALSE;
      foreach ($defaults as $defaultkey => $defaultsetting) {
        $value = variable_get('ubl_access_rights_from_metadata_default_' . $defaultkey);
        if (!array_key_exists($defaultkey, $accesssettings) && !empty($value)) {
          $accesssettings[$defaultkey] = $value;
          $didusedefault = TRUE;
          $accesssettings['_reasons'][$defaultkey] = array('default');
        }
      }
      if ($didusedefault) {
        $accesssettings['_source_of_settings'][] = 'default';
      }
    }
    $settings[$obj->id] = $accesssettings;
  }

  return $settings[$obj->id];
}

function ubl_access_rights_from_metadata_settings_has_all_needed_keys($accesssettings, $neededkeys) {
  if (empty($neededkeys)) {
    return TRUE;
  }
  foreach ($neededkeys as $neededkey) {
    if (!array_key_exists($neededkey, $accesssettings)) {
      return FALSE;
    }
  }
  return TRUE;
}

function ubl_access_rights_from_metadata_default_settings() {
  return array(
    'deny_viewing' => array('title' => t('Deny viewing'), 'description' => t('Deny all viewing access to the whole object, should be TRUE or FALSE.'), 'type' => 'boolean'),
    'deny_access_to_dsid' => array('title' => t('Restricted datastreams'), 'description' => t('Comma seperated list of datastream IDs of datastreams that are restricted.'), 'type' => 'dsids'),
    'allow_access_to_dsid' => array('title' => t('Accessible datastreams'), 'description' => t('Comma seperated list of datastream IDs of datastreams that are accessible.'), 'type' => 'dsids'),
    'provide_download_of_dsid' => array('title' => t('Downloadable datastreams'), 'description' => t('Comma seperated list of datastream IDs that are downloadable.'), 'type' => 'dsids'),
    'access_text' => array('title' => t('Access text'), 'type' => 'text'),
    'access_usetext' => array('title' => t('Access use text'), 'type' => 'text'),
    'access_link' => array('title' => t('Access link'), 'description' => t('A URL to a page explaining the access rights.'), 'type' => 'url'),
    'access_image' => array('title' => t('Access image'), 'description' => t('A URL to an access image.'), 'type' => 'url'),
  );
}
