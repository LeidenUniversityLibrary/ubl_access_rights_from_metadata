<?php

/**
 * @file
 * Handle configuration; Parse and validate ini files.
 *
 *
 *  Copyright 2017 Leiden University Library
 *
 *  This file is part of ubl_access_rights_from_metadata.
 *
 *  ubl_access_rights_from_metadata is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
  $accessconfig = parse_ini_string($inicontent, TRUE);

  if ($accessconfig === FALSE || !is_array($accessconfig)) {
    $error = "Error parsing file '$inifile': not an ini file.";
    return FALSE;
  }
  if (!array_key_exists('rightssource', $accessconfig) || !is_array($accessconfig['rightssource'])) {
    $error = "Error parsing file '$inifile': missing mandatory section 'rightssource'.";
    return FALSE;
  }
  $hasrightsvaluesection = FALSE;
  foreach ($accessconfig as $key => $values) {
    if ($key === 'general') {
      if (!isset($values['ip_range'])) {
        $error = "Section '$key' should contain 'ip_range' keys";
      }
    }
    elseif ($key === 'rightssource') {
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
      $hasrightsvaluesection = TRUE; 
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
  if (!$hasrightsvaluesection) {
    $error = "Error parsing file '$inifile': missing a rights value section.";
    return FALSE;
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
        throw new UBLAccessRightsFromMetadataConfigurationException($error);
      }
      if (is_file($configlocation)) {
        $stat = stat($configlocation);
        $config[$configlocation] = array('_moddate_' => $stat['mtime']);
      }
      $aconfig = array();
      if ($inicontent !== FALSE) {
        $iniarray = parse_ini_string($inicontent, TRUE);
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
              if ($k === 'internal' || $k === 'general') {
                foreach ($iniarray[$k] as $sk => $sv) {
                  if ($sk === 'ip_range') {
                    $ipranges = array();
                    foreach ($iniarray[$k]['ip_range'] as $iprange) {
                      list($minip, $maxip) = explode('-', $iprange);
                      $min = ipToHex($minip);
                      $max = ipToHex($maxip);
                      if ($min && $max) {
                        $ipranges[] = array('min' => $min, 'max' => $max);
                      }
                    }
                    $aconfig['_config'][$k]['ip_ranges'] = $ipranges;
                  }
                  else {
                    $aconfig['_config'][$k][$sk] = $sv;
                  }
                }
              }
              else {
                $aconfig['_config'][$k] = $v;
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

function ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys = NULL) {
  return _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys);
}

function ubl_access_rights_from_metadata_get_all_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys = NULL) {
  $all = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys, 'all');
  $internal = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys, 'internal');
  $external = _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys, 'external');
  
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

function _ubl_access_rights_from_metadata_get_access_rights_settings($accessrightsvalue, $config, $do_otherwise, $neededkeys = NULL, $specifickey = FALSE) {
  $accesssettings = NULL;
  if (isset($config[$accessrightsvalue])) {
    $accesssettings = $config[$accessrightsvalue];
  }
  else {
    if ($do_otherwise && isset($config['otherwise'])) {
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
        $ipranges = $config['_config']['internal']['ip_ranges']; 
        $isinternal = _ubl_access_rights_from_metadata_client_ip_in_range($ipranges);
        if ($isinternal) {
          if (isset($accesssettings['internal'])) {
            $accesssettings = $accesssettings['internal'];
            $accesssettings['ip_dependent'] = TRUE;
            $hasInExternal = TRUE;
          }
        }
        else {
          if (isset($accesssettings['external'])) {
            $accesssettings = $accesssettings['external'];
            $accesssettings['ip_dependent'] = TRUE;
            $hasInExternal = TRUE;
          }
        }
      }
      if (!$hasInExternal && isset($accesssettings['all'])) {
        $accesssettings = $accesssettings['all'];
      }
    }
    if ($neededkeys) {
      // Check if any of the needed keys is available, if not do not return the access settings.
      if (ubl_access_rights_from_metadata_settings_has_any_needed_keys($accesssettings, $neededkeys) === FALSE) {
        $accesssettings = NULL;
      }
    }
  }
  return $accesssettings;
}

function ubl_access_rights_from_metadata_settings_has_any_needed_keys($accesssettings, $neededkeys) {
  $availkeyscount = 0;
  foreach ($neededkeys as $neededkey) {
    if (isset($accesssettings[$neededkey])) {
      $availkeyscount++;
    }
  }
  return ($availkeyscount > 0);
}

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
