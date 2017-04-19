<?php

/**
 * @file
 * Handle configuration; Parse and validate ini files.
 */

function ubl_access_rights_from_metadata_valid_config_file($inifile, &$error) {
  $error = '';
  $accessconfig = parse_ini_file($inifile, TRUE);

  if ($accessconfig === FALSE || !is_array($accessconfig)) {
    $error = "Error parsing file '$inifile'.";
    return FALSE;
  }
  foreach ($accessconfig as $key => $values) {
    if ($key === 'rightssource') {
      if (!isset($values['dsid'])) {
        $error = "No key 'dsid' found in '$key'";
      }
      if (!isset($values['xpath'])) {
        $error = "No key 'xpath' found in '$key'";
      }
    }
    elseif ($key === 'internal') {
      if (!is_array($values) || count($values) != 1 || !isset($values['ip_range'])) {
        $error = "Section '$key' should only contain 'ip_range' keys";
      }
    }
    elseif (has_prefix($key, 'all::') || has_prefix($key, 'internal::') || has_prefix($key, 'external::')) {
      if (!array($values) || count($values) == 0) {
        $error = "Section '$key' should contain at least 1 key";
      }
      else {
        foreach (array('deny_access_to_dsid','allow_access_to_dsid','provide_download_of_dsid','allow_access_for_role') as $k) {
          if (isset($values[$k]) && !is_array($values[$k])) {
            $error = "Section '$key' contains key '$k' that is not an array, use '$k\[]'.";
          }
        }
      }
    }
    else {
      $error = "Unexpected section '$key'";
    }
    if ($error) {
      return FALSE;
    }
  } 

  return TRUE;
}

function ubl_access_rights_from_metadata_valid_config_directory($configdir, &$error) {
  $accessprefix = 'access_rights';
  $accesspostfix = '.ini';
  if ($dh = @opendir($configdir)) {
    $foundaccessini = FALSE;
    $accessini = $accessprefix . $accesspostfix;
    while (FALSE !== ($filename = readdir($dh))) {
      if (has_prefix($filename, $accessprefix) && has_postfix($filename, $accesspostfix)) {
        $filepath = $configdir . DIRECTORY_SEPARATOR . $filename;
        $valid = ubl_access_rights_from_metadata_valid_config_file($filepath, $error);
        if (!$valid) {
          return FALSE;
        }
        if ($filename === $accessini) {
          $foundaccessini = TRUE;
        }
      }
    }
    if (!$foundaccessini) {
      $error = "No mandatory file '$accessini' found in directory '$configdir'";
      return FALSE;
    }
  }
  else {
    $error = "Cannot open directory '$configdir'";
    return FALSE;
  }
  return TRUE;
}

function ubl_access_rights_from_metadata_get_configuration($options) {
  $config = &drupal_static(__FUNCTION__);
  if (isset($options['directory'])) {
    $configdir = $options['directory'];
  }
  else {
    $configdir = drupal_realpath("public://ubl_access_rights_from_metadata");
  }
  if (!isset($config[$configdir])) {
    $stat = stat($configdir);
    $cachename = $configdir;
    $cachebin = 'cache_ubl_access_rights_from_metadata_config';
    if ($cache = cache_get($cachename, $cachebin)) {
      $data = $cache->data;
      if ($data['_moddate_'] === $stat['mtime']) {
        $config[$configdir] = $cache->data;
      }
    }
    if (!isset($config[$configdir])) {
      $config[$configdir] = array('_moddate_' => $stat['mtime']);
      if ($dh = @opendir($configdir)) {
        $accessprefix = 'access_rights';
        $accesspostfix = '.ini';
        while (FALSE !== ($filename = readdir($dh))) {
          if (has_prefix($filename, $accessprefix) && has_postfix($filename, $accesspostfix)) {
            $filepath = $configdir . DIRECTORY_SEPARATOR . $filename;
            $aconfig = array();
            $iniarray = parse_ini_file($filepath, TRUE);
            if ($iniarray) {
              foreach ($iniarray as $k => $v) {
                $parts = explode('::', $k, 2);
                if (count($parts) == 2) {
                  list($location,$accessvalue) = $parts;
                  if (!isset($aconfig[$accessvalue])) {
                    $aconfig[$accessvalue] = array(); 
                  }
                  $aconfig[$accessvalue][$location] = $v;
                }
                else {
                  if (!isset($aconfig['_config'])) {
                    $aconfig['_config'] = array();
                  }
                  if ($k === 'internal') {
                    $ipranges = array();
                    foreach ($iniarray[$k]['ip_range'] as $iprange) {
                      list($minip, $maxip) = explode('-', $iprange);
                      $min = ipToHex($minip);
                      $max = ipToHex($maxip);
                      if ($min && $max) {
                        $ipranges[] = array('min' => $min, 'max' => $max);
                      }
                    }
                    $aconfig['_config'][$k] = array('ip_ranges' => $ipranges);
                  }
                  else {
                    $aconfig['_config'][$k] = $v;
                  }
                }
              } 
            }
            $type = substr($filename, strlen($accessprefix) + 1, -strlen($accesspostfix));
            $config[$configdir][$type] = $aconfig;
          }
        }
      }
      cache_set($cachename, $config[$configdir], $cachebin);
    }
  }
  if (isset($options['file'])) {
    $type = substr($options['file'], strlen($accessprefix) + 1, -strlen($accesspostfix));
    return $config[$configdir][$type];
  }
  elseif (isset($options['object'])) {
    if (count($config[$configdir]) > 1) {
      $objconfig = _ubl_access_rights_from_metadata_config_for_object($config[$configdir], $options['object']);
      if ($objconfig) {
        return $objconfig;
      }
    }
  }
  return $config[$configdir][FALSE];
}

function _ubl_access_rights_from_metadata_unfold_dsid_array($cnfg, $key, $obj, $dontmessdsids = array()) {
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


function _ubl_access_rights_from_metadata_config_for_object($config, $obj) {
  module_load_include('inc', 'islandora', 'includes/utilities');

  list ($namespace, $shortpid) = explode(':', $obj->id, 2);
  if (isset($config[$namespace . '_' . $shortpid])) {
    return $config[$namespace . '_' . $shortpid];
  }
  elseif (isset($config[$namespace])) {
    return $config[$namespace];
  }

  try {
    $relations = array_merge(
      $obj->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection'),
      $obj->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf'),
      $obj->relationships->get(FEDORA_RELS_EXT_URI, 'isConstituentOf'));
  }
  catch (RepositoryException $e) {
    // @todo some logging would be nice, not sure what this throws.
    return FALSE;
  }
  $map = function($o) {
    return islandora_object_load($o['object']['value']);
  };
  $relations = array_filter(array_map($map, $relations));

  foreach ($relations as $relation) {
    $found = _ubl_access_rights_from_metadata_config_for_object($config, $relation);
    if ($found) {
      return $found;
    }
  }
  return FALSE;
}

function ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config) {
  return _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config);
}

function ubl_access_rights_from_metadata_get_all_access_rights_settings($accessrightsvalue, $config) {
  $all = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, 'all');
  $internal = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, 'internal');
  $external = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, 'external');
  
  if (!is_array($all)) {
    $all = array();
  }
  if (!is_array($internal)) {
    $internal = array();
  }
  if (!is_array($external)) {
    $external = array();
  }
  return array_merge($external, $internal, $all);
}

function _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $specifickey = FALSE) {
  $accesssettings = NULL;
  if (isset($config[$accessrightsvalue])) {
    $accesssettings = $config[$accessrightsvalue];
  }
  else {
    if (isset($config['otherwise'])) {
      $accesssettings = $config['otherwise'];
    }
  }
  if (is_array($accesssettings)) {
    if ($specifickey) {
      $accesssettings = isset($accesssettings[$specifickey])?$accesssettings[$specifickey]:NULL;
    }
    else {
      $hasInExternal = FALSE;
      if (isset($accesssettings['internal']) || isset($accesssettings['external'])) {
        $isinternal = FALSE;
        $clientip = ip_address();
        $clientiphex = ipToHex($clientip);
        if ($clientiphex) {
          $ipranges = $config['_config']['internal']['ip_ranges']; 
            foreach ($ipranges as $iprange) {
            $min = $iprange['min'];
            $max = $iprange['max'];
            if (strcmp($min, $clientiphex) <= 0 && strcmp($clientiphex, $max) <= 0) {
              $isinternal = TRUE;
              continue;
            }
          }
        }
        if ($isinternal) {
          if (isset($accesssettings['internal'])) {
            $accesssettings = $accesssettings['internal'];
            $hasInExternal = TRUE;
          }
        }
        else {
          if (isset($accesssettings['external'])) {
            $accesssettings = $accesssettings['external'];
            $hasInExternal = TRUE;
          }
        }
      }
      if (!$hasInExternal && isset($accesssettings['all'])) {
        $accesssettings = $accesssettings['all'];
      }
    }
  }
  return $accesssettings;
}

function ubl_access_rights_from_metadata_get_rights_source_settings($config) {
  if (isset($config['_config']['rightssource'])) {
    return $config['_config']['rightssource'];
  }
  return array();
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
