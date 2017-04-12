<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);

/**
 * @file
 * Change XACML based on access rights configuration.
 */

function ubl_access_rights_from_metadata_xacml_modify_if_needed($obj, $config) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/config');

  $accessrightsvalue = ubl_access_rights_from_metadata_get_access_rights_value($obj, $config);

  $accesssettings = ubl_access_rights_from_metadata_get_settings($accessrightsvalue, $config);

  if (isset($accesssettings)) {
    ubl_access_rights_from_metadata_xacml_modify_with_settings($obj, $accesssettings);

    // Apply the access rights also to children of this object that don't have access rights themselves.
    ubl_access_rights_from_metadata_xacml_modify_children($obj, $config, $accesssettings);
  }
}

function ubl_access_rights_from_metadata_xacml_modify_children($obj, $config, $accesssettings) {
  module_load_include('inc', 'ubl_access_rights_from_metadata', 'includes/access_rights_value');

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

  if (isset($accesssettings['all'])) {
    $denydsids = isset($accesssettings['all']['deny_access_to_dsid'])?$accesssettings['all']['deny_access_to_dsid']:array();
    $denydsids = _unfold_dsid_array($denydsids, $obj, array('RELS-INT','RELS-EXT','POLICY'));
    $allowdsids = isset($accesssettings['all']['allow_access_to_dsid'])?$accesssettings['all']['allow_access_to_dsid']:array();
    $allowdsids = _unfold_dsid_array($allowdsids, $obj);
    $allowedroles = isset($accesssettings['all']['allow_access_for_role'])?$accesssettings['all']['allow_access_for_role']:array();

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
  }

  if ($xacml && $haschanges) {
    $xacml->writeBackToFedora();
  }
}

