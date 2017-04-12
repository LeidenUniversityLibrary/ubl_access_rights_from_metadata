<?php

/**
 * @file
 * Retrieve the access rights value from an object, using caching.
 */

function ubl_access_rights_from_metadata_get_access_rights_value($obj, $config, $rebuildcache = FALSE) {
  $id2acvalue = &drupal_static(__FUNCTION__, array());
  $id = $obj->id;
  if (!array_key_exists($id, $id2acvalue) || $rebuildcache) {
    $cachename = $id;
    $cachebin = 'cache_ubl_access_rights_from_metadata_access_rights_value';
    if ($cache = cache_get($cachename, $cachebin)) {
      $id2acvalue[$id] = $cache->data;
    }
    if (!array_key_exists($id, $id2acvalue) || $rebuildcache) {
      $dsid = $config['_config']['rightssource']['dsid'];
      if (isset($obj[$dsid])) {
        $xpath = $config['_config']['rightssource']['xpath'];
        $content = $obj[$dsid]->content;
        $domdoc = new DOMDocument();
        if ($domdoc->loadXML($content)) {
          $id2acvalue[$id] = ''; // subtle change to indicate that the DS XML exists, but does not have any access rights info
          $domxpath = new DOMXPath($domdoc);
          foreach ($config['_config']['rightssource']['namespace'] as $ns) {
            list($prefix, $uri) = explode(' ', $ns, 2);
            $domxpath->registerNamespace($prefix, $uri);
          }
          $domnodelist = $domxpath->query($xpath);
          if ($domnodelist && $domnodelist->length > 0) {
            foreach ($domnodelist as $domnode) {
              $text = $domnode->textContent;
              if (isset($text) && strlen($text) > 0) {
                $id2acvalue[$id] = $text;
                break;
              }
            }
          }
        }
      }
      if (!array_key_exists($id, $id2acvalue)) {
        $id2acvalue[$id] = NULL;
      }
      cache_set($cachename, $id2acvalue[$id], $cachebin);
    }
  }
  return $id2acvalue[$id];
}
