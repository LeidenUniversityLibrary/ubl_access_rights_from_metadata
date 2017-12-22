<?php

/**
 * @file
 * Functions for user interface access rights.
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

function ubl_access_rights_from_metadata_accessible_dsids($obj, &$realobjid = NULL) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_settings');

  $configlocation = variable_get('ubl_access_rights_from_metadata_config_location');
  $config = ubl_access_rights_from_metadata_get_configuration($configlocation);
  
  $neededkeys = array('allow_access_to_dsid','deny_access_to_dsid');
  $accesssettings = ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config, $neededkeys);

  $accessibledsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of allowed dsids if it is in the deny dsids also.
  $accessibledsids = array_diff($accessibledsids, $denydsids);

  if ($realobjid !== NULL) {
    // TODO
    if ($obj->id !== $accesssettings['_source_of_settings'][0]) {
      $realobjid = $accesssettings['_source_of_settings'][0];
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

  $accessibledsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $denydsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj);

  // remove any occurence of deny dsids if it is in the allowed dsids also.
  $restricteddsids = array_diff($denydsids, $accessibledsids);

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['_source_of_settings'][0]) {
      $realobjid = $accesssettings['_source_of_settings'][0];
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
   if ($obj->id !== $accesssettings['_source_of_settings'][0]) {
      $realobjid = $accesssettings['_source_of_settings'][0];
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

  if ($realobjid !== NULL) {
   if ($obj->id !== $accesssettings['_source_of_settings'][0]) {
      $realobjid = $accesssettings['_source_of_settings'][0];
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
   if ($obj->id !== $accesssettings['_source_of_settings'][0]) {
      $realobjid = $accesssettings['_source_of_settings'][0];
    }
  }

  return $result;
}

function ubl_access_rights_from_metadata_get_access_settings_from_object_or_parent($obj, $config) {

  $settings = &drupal_static(__FUNCTION__);

  if (!isset($settings[$obj->id])) {
    module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');
    module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_settings');
  
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
              $accesssettings['_conditions_met'] = array_unique(array_merge($accesssettings['_conditions_met'], $paccesssettings['_conditions_met']));
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
        }
      }
      if ($didusedefault) {
        $accesssettings['_source_of_settings'][] = 'default';
      }
    }
dpm($accesssettings, "access settings from object " . $obj->id);
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
    'deny_access_to_dsid' => array('title' => t('Restricted datastreams'), 'description' => t('Comma seperated list of datastream IDs of datastreams that are restricted.'), 'type' => 'dsids'),
    'allow_access_to_dsid' => array('title' => t('Accessible datastreams'), 'description' => t('Comma seperated list of datastream IDs of datastreams that are accessible.'), 'type' => 'dsids'),
    'provide_download_of_dsid' => array('title' => t('Downloadable datastreams'), 'description' => t('Comma seperated list of datastream IDs that are downloadable.'), 'type' => 'dsids'),
    'access_text' => array('title' => t('Access text'), 'type' => 'text'),
    'access_usetext' => array('title' => t('Access use text'), 'type' => 'text'),
    'access_link' => array('title' => t('Access link'), 'description' => t('A URL to a page explaining the access rights.'), 'type' => 'url'),
    'access_image' => array('title' => t('Access image'), 'description' => t('A URL to an access image.'), 'type' => 'url'),
  );
}
