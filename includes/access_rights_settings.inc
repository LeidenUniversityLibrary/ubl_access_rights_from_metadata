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
  $conditionsmet = array();
  $isipdependent = FALSE;

  foreach ($conditionconfig as $conditionname => $conditionvalues) {
dpm($conditionvalues, $conditionname);
    $conditions[$conditionname] = _ubl_access_rights_from_metadata_evaluate_condition($conditionvalues, $object, $conditions, $config);
    if ($conditions[$conditionname]) {
      if (!$isipdependent && _ubl_access_rights_from_metadata_is_ip_dependent($conditionname, $config)) {
        $isipdependent = TRUE;
      }
    }
  }
  
  $accessconfig = $config['access'];
  foreach ($accessconfig as $accessname => $accessvalues) {
    if (isset($conditions[$accessname]) && $conditions[$accessname]) {
      $conditionsmet[] = $accessname;
      $accessrights = $accessrights + $accessvalues;
    }
  }
  if (!empty($accessrights)) {
    $accessrights['_conditions_met'] = $conditionsmet;
  }
  $accessrights['_ip_dependent'] = $isipdependent;

dpm($conditions, 'conditions for object '. $object->id);
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
dpm(array('isip' => $isipdependent, 'config' => $config['condition'][$conditionname]), $conditionname);
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
dpm(array('trues' => $trues, 'results' => $results, 'result' => $result), $object->id);
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


/*

function _ubl_access_rights_from_metadata_client_ip_in_range($ipranges) {
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
  return $isinrange;
}

function ubl_access_rights_from_metadata_is_request_from_server($object) {
  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  if (!$configlocation) {
    return;
  }
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  $ipranges = $config['_config']['general']['ip_ranges']; 
  return _ubl_access_rights_from_metadata_client_ip_in_range($ipranges);
}

function ubl_access_rights_from_metadata_get_rights_source_settings($config) {
  if (isset($config['_config']['rightssource'])) {
    return $config['_config']['rightssource'];
  }
  return array();
}

*/

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

