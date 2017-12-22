<?php

/**
 * @file
 * Handle configuration; Parse and validate ini files.
 */

function ubl_access_rights_from_metadata_valid_config_file($inifile, &$error) {
  $error = '';
  $inicontent = @file_get_contents($inifile);

  if (($inicontent === FALSE) || (strlen($inicontent) == 0)) {
    $error = "Error reading file '$inifile'.";
    return FALSE;
  }
  return _ubl_access_rights_from_metadata_valid_config_content($inicontent, $inifile, $error);
}
  
function _ubl_access_rights_from_metadata_valid_config_content($inicontent, $inifile, &$error) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_settings');

  $accessconfig = parse_ini_string($inicontent, TRUE);

  if ($accessconfig === FALSE || !is_array($accessconfig)) {
    $error = "Error parsing file '$inifile': not an ini file.";
    return FALSE;
  }
  $sections = array('metadata' => array(), 'condition' => array(), 'access' => array());
  foreach ($accessconfig as $key => $values) {
    $keyparts = explode(':', $key);
    if (count($keyparts) != 2) {
      $error = t("Section '@section' is invalid, should have format as 'type:value'.", array("@section" => $key));
      return FALSE;
    }
    $sectiontype = $keyparts[0];
    $sectionname = $keyparts[1];
    if (has_prefix($key, 'metadata:')) {
      $mandatory = array('datastream' => '/^[A-Z0-9]+$/', 'xpath' => '');
      $optional = array('namespace' => array());
      if (!_ubl_access_rights_from_metadata_valid_subkeys($values, $key, $mandatory, $optional, $error)) {
        return FALSE;
      }
      $sections['metadata'][] = $sectionname;
    }
    elseif (has_prefix($key, 'condition:')) {
      $mandatory = array();
      $optional = array(
        'user_role' => array(),
        'ip_range' => array(),
        'condition' => array(),
        'operator' => '/^(negate|and|or)$/',
        'metadata' => '',
        'comparator' => '',
        'value' => '',
      );
      if (!_ubl_access_rights_from_metadata_valid_subkeys($values, $key, $mandatory, $optional, $error)) {
        return FALSE;
      }
      if (isset($values['operator'])) {
        $operator = $values['operator'];
        unset($values['operator']);
      }
      if (isset($values['condition'])) {
        $conditions = $values['condition'];
        unset($values['condition']);
      }
      $mdcomparevalue = array('metadata' => 1, 'comparator' => 1, 'value' => 1);
      $intersect = array_intersect_key($values, $mdcomparevalue);
      if (count($intersect) != 0 && count($intersect) != count($mdcomparevalue)) {
        $error = t("Condition '@section' is invalid, keys 'metadata', 'comparator' and 'value' should all be there or none at all, possibly combined with keys 'operator' and/or 'condition'.", array("@section" => $key));
        return FALSE;
      }
      elseif (isset($values['metadata']) && !in_array($values['metadata'], $sections['metadata'])) {
        $error = t("Condition '@section' has key 'metadata' = '@value' that is not defined in the section metadata.", array("@section" => $key, "@value" => $values['metadata']));
        return FALSE;
      }
      elseif (isset($values['comparator'])) {
        $prefix = '_ubl_access_rights_from_metadata_comparator_';
        $compfunc = $prefix . $values['comparator'];
        if (!function_exists($compfunc)) {
          $allfuncs = get_defined_functions();
          $comparators = array_filter($allfuncs['user'], function($v) { return (substr($v, 0, strlen($prefix)) === $prefix); });
          $error = t("Comparator '@comparator' in section '@section' is not valid. Use any of these: @comparators", array("@section" => $key, "@comparator" => $values['comparator'], "@comparators" => implode(", ", $comparators)));
          return FALSE;
        }
      }
      if (isset($values['ip_range'])) {
        foreach ($values['ip_range'] as $iprange) {
          $parts = explode('-', $iprange);
          if (count($parts) !== 2) {
            $error = t("IP range '@iprange' in section '@section' is not valid. Should be like 1.1.1.1-2.2.2.2", array("@section" => $key, "@iprange" => $iprange));
            return FALSE;
          }
          $min = ipToHex($parts[0]);
          $max = ipToHex($parts[1]);
          if (!$min) {
            $error = t("IP range '@iprange' (first value) in section '@section' is not valid. Should be like 1.1.1.1-2.2.2.2", array("@section" => $key, "@iprange" => $iprange));
            return FALSE;
          }
          if (!$max) {
            $error = t("IP range '@iprange' (second value) in section '@section' is not valid. Should be like 1.1.1.1-2.2.2.2", array("@section" => $key, "@iprange" => $iprange));
            return FALSE;
          }
        }
      }
      $sections['condition'][] = $sectionname;
    }
    elseif (has_prefix($key, 'access:')) {
      $mandatory = array();
      $optional = array(
        'deny_access_to_dsid' => array(),
        'allow_access_to_dsid' => array(),
        'provide_download_of_dsid' => array(),
        'access_text' => '',
        'access_usetext' => '',
        'access_link' => '/^https?:.*/',
        'access_image' => '/^https?:.*/',
      );
      if (!_ubl_access_rights_from_metadata_valid_subkeys($values, $key, $mandatory, $optional, $error)) {
        return FALSE;
      }
      $sections['access'][] = $sectionname;
    }
    else {
      $error = "Unexpected section '$key'";
    }
    if ($error) {
      return FALSE;
    }
  }
  foreach ($sections as $key => $values) {
    if (count($values) == 0) {
      $error = t("Missing section '@section'.", array('@section' => $key));
      return FALSE;
    }
  }
  foreach ($sections['access'] as $sectionname) {
    if ($sectionname !== 'default' && !in_array($sectionname, $sections['condition'])) {
      $error = t("Section 'access:@section' does not have a corresponding 'condition:@section'.", array('@section' => $sectionname));
      return FALSE;
    }
  }
  if (in_array('default', $sections['condition'])) {
    $error = t("Section 'condition:default' should not exist.");
    return FALSE;
  }

  return TRUE;
}

function _ubl_access_rights_from_metadata_valid_subkeys($config, $section, $mandatory, $optional, &$error) {
  $additional = array_diff_key($config, $mandatory, $optional);
  foreach ($additional as $ak => $av) {
    $error = t("Section '@section' contains an additional key '@key' (with value '@value') that should not be there.", array("@section" => $section, "@key" => $ak, "@value" => is_array($config[$ak])?implode(",", $config[$ak]):(string)$config[$ak]));
    return FALSE;
  }
  $missing = array_diff_key($mandatory, $config);
  foreach ($missing as $ak => $av) {
    $error = t("Section '@section' is missing mandatory key '@key'.", array("@section" => $section, "@key" => $ak));
    return FALSE;
  }
  $all = $mandatory + $optional;
  foreach ($all as $k => $v) {
    if (isset($config[$k])) {
      if (is_array($config[$k]) && !is_array($all[$k])) {
        $error = t("Section '@section' contains key '@key' with array as value, but should be single value.", array("@section" => $section, "@key" => $k));
        return FALSE;
      }
      if (!is_array($config[$k]) && is_array($all[$k])) {
        $error = t("Section '@section' contains key '@key' with single value, but should be array value.", array("@section" => $section, "@key" => $k));
        return FALSE;
      }
      if (!is_bool($config[$k]) && is_bool($all[$k])) {
        $error = t("Section '@section' contains key '@key' with value that should be a boolean value.", array("@section" => $section, "@key" => $k));
        return FALSE;
      }
      if (is_string($config[$k]) && strlen($all[$k]) > 0 && preg_match($all[$k], $config[$k]) === 0) {
        $error = t("Section '@section' contains key '@key' with incorrect value '$@value'.", array("@section" => $section, "@key" => $k, "@value" => $v));
        return FALSE;
      }
    }
  }
  return TRUE;
}

function ubl_access_rights_from_metadata_get_configuration($configlocation) {
  $config = &drupal_static(__FUNCTION__);

  if (!isset($config[$configlocation])) {
    $cachename = $configlocation;
    $cachebin = 'cache_ubl_access_rights_from_metadata_config';
    if ($cache = cache_get($cachename, $cachebin)) {
      $data = $cache->data;
      if (is_file($configlocation)) {
        $stat = stat($configlocation);
        if ($data['_moddate_'] === $stat['mtime']) {
          $config[$configlocation] = $data;
        }
      }
      else {
        $config[$configlocation] = $data;
      }
    }
    if (!isset($config[$configlocation])) {
      $inicontent = @file_get_contents($configlocation);
      $error = '';
      $isvalid = _ubl_access_rights_from_metadata_valid_config_content($inicontent, 'access config', $error);
      if (!$isvalid) {
        // config is not valid; the only save thing we can do is bail...
        $error = "Error when reading configuration of 'access rights from metadata': $error";
        throw new RuntimeException($error);
      }
      $aconfig = array();
      if (is_file($configlocation)) {
        $stat = stat($configlocation);
        $aconfig = array('_moddate_' => $stat['mtime']);
      }
      if ($inicontent !== FALSE) {
        $iniarray = parse_ini_string($inicontent, TRUE);
        if ($iniarray) {
          foreach ($iniarray as $sk => $vs) {
            list($type, $name) = explode(':', $sk, 2);
            if (!isset($aconfig[$type])) {
              $aconfig[$type] = array();
            }
            if (!isset($aconfig[$type][$name])) {
              $aconfig[$type][$name] = array();
            }
            foreach ($vs as $k => $v) {
              if ($k === 'ip_range') {
                foreach ($v as $iprange) {
                  list($minip, $maxip) = explode('-', $iprange);
                  $min = ipToHex($minip);
                  $max = ipToHex($maxip);
                  if ($min && $max) {
                    $ipranges[] = array('min' => $min, 'max' => $max);
                  }
                }
                $aconfig[$type][$name][$k] = $ipranges;
              }
              else {
                $aconfig[$type][$name][$k] = $v;
              }
            } 
          } 
        }
      }
      $config[$configlocation] = $aconfig;
      $expire_after = time() + (60*60*24); // keep in cache for about a day.
      cache_set($cachename, $config[$configlocation], $cachebin, $expire_after);
    }
  }
  return $config[$configlocation];
}

function ubl_access_rights_from_metadata_refresh_configuration($configlocation) {
  $cachename = $configlocation;
  $cachebin = 'cache_ubl_access_rights_from_metadata_config';
  cache_clear_all($cachename, $cachebin); 
}

function has_prefix($str, $prefix) {
  return (substr($str, 0, strlen($prefix)) === $prefix);
}

function has_postfix($str, $postfix) {
  return (substr($str, -strlen($postfix)) === $postfix);
}

function ipToHex($ipAddress) {
  // function by johniskew at yahoo dot com.
  // don't use ip2long because it cannot handle ip v6.
  $hex = '';
  if(strpos($ipAddress, ',') !== false) {
    $splitIp = explode(',', $ipAddress);
    $ipAddress = trim($splitIp[0]);
  }
  $isIpV6 = false;
  $isIpV4 = false;
  if(filter_var($ipAddress, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6) !== false) {
    $isIpV6 = true;
  }
  else if(filter_var($ipAddress, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4) !== false) {
    $isIpV4 = true;
  }
  if(!$isIpV4 && !$isIpV6) {
    return false;
  }
  // IPv4 format
  if($isIpV4) {
    $parts = explode('.', $ipAddress);
    for($i = 0; $i < 4; $i++) {
      $parts[$i] = str_pad(dechex($parts[$i]), 2, '0', STR_PAD_LEFT);
    }
    $ipAddress = '::'.$parts[0].$parts[1].':'.$parts[2].$parts[3];
    $hex = join('', $parts);
  }
  // IPv6 format
  else {
    $parts = explode(':', $ipAddress);
    // If this is mixed IPv6/IPv4, convert end to IPv6 value
    if(filter_var($parts[count($parts) - 1], FILTER_VALIDATE_IP, FILTER_FLAG_IPV4) !== false) {
      $partsV4 = explode('.', $parts[count($parts) - 1]);
      for($i = 0; $i < 4; $i++) {
        $partsV4[$i] = str_pad(dechex($partsV4[$i]), 2, '0', STR_PAD_LEFT);
      }
      $parts[count($parts) - 1] = $partsV4[0].$partsV4[1];
      $parts[] = $partsV4[2].$partsV4[3];
    }
    $numMissing = 8 - count($parts);
    $expandedParts = array();
    $expansionDone = false;
    foreach($parts as $part) {
      if(!$expansionDone && $part == '') {
        for($i = 0; $i <= $numMissing; $i++) {
          $expandedParts[] = '0000';
        }
        $expansionDone = true;
      }
      else {
        $expandedParts[] = $part;
      }
    }
    foreach($expandedParts as &$part) {
      $part = str_pad($part, 4, '0', STR_PAD_LEFT);
    }
    $ipAddress = join(':', $expandedParts);
    $hex = join('', $expandedParts);
  }
  // Validate the final IP
  if(!filter_var($ipAddress, FILTER_VALIDATE_IP)) {
    return false;
  }
  return strtolower(str_pad($hex, 32, '0', STR_PAD_LEFT));
}

