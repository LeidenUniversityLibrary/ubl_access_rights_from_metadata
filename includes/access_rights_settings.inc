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

