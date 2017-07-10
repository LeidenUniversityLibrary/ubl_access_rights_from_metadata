/**
 * @file
 * js/ubl_access_rights_from_metadata.js
 */

jQuery(document).ready(function() {
  var replaceInaccessibleThumbnail = function() {
    var parts = this.src.split("?", 2);
    if (parts[0].endsWith('datastream/TN/view')) {
      var newsrc = '/sites/all/modules/ubl_access_rights_from_metadata/images/tn_restricted_access.gif';
      if (parts.length == 2) {
        newsrc += '?' + parts[1];
      }
      
      jQuery(this)
        .unbind("error")
        .attr("src", newsrc); 
    }
  };
  jQuery('.islandora-solr-search-result IMG')
    .on("error", replaceInaccessibleThumbnail)
    .each(function() {
      if (this.complete) { 
        alert('complete: ' + this);
      }
    });
});
