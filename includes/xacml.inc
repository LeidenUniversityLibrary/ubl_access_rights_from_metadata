<?php

/**
 * @file
 * Change XACML based on access rights configuration.
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

function ubl_access_rights_from_metadata_xacml_modify_if_needed($obj, $config) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $accessrightsvalue = ubl_access_rights_from_metadata_get_access_rights_value($obj, $config);

  $accesssettings = ubl_access_rights_from_metadata_get_all_access_rights_settings($accessrightsvalue, $config, TRUE, array('deny_access_to_dsid', 'allow_access_to_dsid', 'allow_access_for_role'));

  if (isset($accesssettings)) {
    ubl_access_rights_from_metadata_xacml_modify_with_settings($obj, $accesssettings);

    // Apply the access rights also to children of this object that don't have access rights themselves.
    ubl_access_rights_from_metadata_xacml_modify_children($obj, $config, $accesssettings);
  }
}

function ubl_access_rights_from_metadata_xacml_modify_children($obj, $config, $accesssettings) {

  if (in_array('islandora:bookCModel', $obj->models)) {
    if (module_load_include('inc', 'islandora_paged_content', 'includes/utilities')) {
      $pages = islandora_paged_content_get_pages($obj);
      foreach ($pages as $pagepid => $pageinfo) {
        $childobj = islandora_object_load($pagepid);
        $childaccessrightsvalue = ubl_access_rights_from_metadata_get_access_rights_value($childobj, $config);
        if (!$childaccessrightsvalue) {
          ubl_access_rights_from_metadata_xacml_modify_with_settings($childobj, $accesssettings);
        }
      }
    }
  }
  if (in_array('islandora:compoundCModel', $obj->models)) {
    $compoundparts = islandora_compound_object_get_parts($obj->id);
    foreach ($compoundparts as $partpid) {
      $childobj = islandora_object_load($partpid);
      $childaccessrightsvalue = ubl_access_rights_from_metadata_get_access_rights_value($childobj, $config);
      if (!$childaccessrightsvalue) {
        ubl_access_rights_from_metadata_xacml_modify_with_settings($childobj, $accesssettings);
        ubl_access_rights_from_metadata_xacml_modify_children($childobj, $config, $accesssettings);
      }
    }
  }
}

function ubl_access_rights_from_metadata_xacml_modify_with_settings($obj, $accesssettings) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $xacml = NULL;
  $haschanges = FALSE;

  $denydsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'deny_access_to_dsid', $obj, array('RELS-INT','RELS-EXT','POLICY'));
  $allowdsids = ubl_access_rights_from_metadata_unfold_dsid_array($accesssettings, 'allow_access_to_dsid', $obj);
  $allowedroles = isset($accesssettings['allow_access_for_role'])?$accesssettings['allow_access_for_role']:array();

  // Remove the allowed dsids from the denied dsids.
  $denydsids = array_diff($denydsids, $allowdsids);

  $xacml = new IslandoraXacml($obj);
  // Alter the denied DSIDs, but first check if a change is needed.
  $currentdsids = $xacml->datastreamRule->getDsids();
  if (count(array_diff($denydsids, $currentdsids)) > 0) {
    $xacml->datastreamRule->addDsid($denydsids);
    $haschanges = TRUE;
    $currentdsids = $xacml->datastreamRule->getDsids();
  }
  // Make sure that the allowed dsids are not in the disallowed datastream rule.
  if (count(array_intersect($allowdsids, $currentdsids)) > 0) {
    $xacml->datastreamRule->removeDsid($allowdsids);
    $haschanges = TRUE;
  }
  // Alter the allowed roles, but first check if a change is needed.
  $currentroles = $xacml->datastreamRule->getRoles();
  if (count(array_diff($allowedroles, $currentroles)) > 0) {
    $xacml->datastreamRule->addRole($allowedroles);
    $haschanges = TRUE;
  }

  if ($xacml && $haschanges) {
    $xacml->writeBackToFedora();
  }
}

