<?php

/**
 * @file
 * Retrieve the metadata value from an object, using caching.
 */

function ubl_access_rights_from_metadata_get_metadata_value($obj, $config, $neededmdkey, $rebuildcache = FALSE) {
  $id2mdvalue = &drupal_static(__FUNCTION__, array());
  $id = $obj->id;
  $lastchangedate = (string)$obj->lastModifiedDate;
  if ($rebuildcache) {
    unset($id2mdvalue[$id]);
  }
  if (!isset($id2mdvalue[$id])) {
    $cachename = $id;
    $cachebin = 'cache_ubl_access_rights_from_metadata_metadata_value';
    if (!$rebuildcache) {
      if ($cache = cache_get($cachename, $cachebin)) {
        $found = $cache->data;
        if ($found['_date'] === $lastchangedate) {
          $id2mdvalue[$id] = $found;
        }
      }
    }
    if (!isset($id2mdvalue[$id])) {
      $id2mdvalue[$id] = array('_date' => $lastchangedate);
      $mdsettings = $config['metadata'];
      foreach ($mdsettings as $mdkey => $mdvalues) {
        $dsid = $mdvalues['datastream'];
        $id2mdvalue[$id][$mdkey] = NULL;
        if (isset($obj[$dsid])) {
          $xpaths = $mdvalues['xpath'];
          $remove = array();
        
          $content = $obj[$dsid]->content;
          $domdoc = new DOMDocument();
          if ($domdoc->loadXML($content)) {
            $id2mdvalue[$id][$mdkey] = ''; // subtle change to indicate that the DS XML exists, but does not have the metadata value
            $domxpath = new DOMXPath($domdoc);
            if (isset($mdvalues['namespace'])) {
              foreach ($mdvalues['namespace'] as $ns) {
                list($prefix, $uri) = explode(' ', $ns, 2);
                $domxpath->registerNamespace($prefix, $uri);
              }
            }
            $xpath = $mdvalues['xpath'];
            $domnodelist = $domxpath->query($xpath);
            if ($domnodelist && $domnodelist->length > 0) {
              foreach ($domnodelist as $domnode) {
                $text = $domnode->textContent;
                if (isset($text) && strlen($text) > 0) {
                  $id2mdvalue[$id][$mdkey] = $text;
                }
              }
            }
          }
        }
      }
      cache_set($cachename, $id2mdvalue[$id], $cachebin);
    }
  }
  return $id2mdvalue[$id][$neededmdkey];
}
