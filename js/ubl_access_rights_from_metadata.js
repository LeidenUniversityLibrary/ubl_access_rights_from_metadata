/**
 * @file
 * js/ubl_access_rights_from_metadata.js
 */

(function ($) {
  var tnurl = '';
  Drupal.behaviors.ubl_access_rights_from_metadata = {
    attach: function(context, settings) {
      tnurl = settings.ubl_access_rights_from_metadata.tn_access_restricted;
    }
  }
  var replaceInaccessibleThumbnail = function() {
    var _replaceInaccessibleThumbnail = function(img) {
      if (tnurl === '') {
        setTimeout(function() { _replaceInaccessibleThumbnail(img) }, 100);
        return;
      }
      var parts = img.src.split("?", 2);
      if (parts[0].endsWith('datastream/TN/view')) {
        var newsrc = settings.ubl_access_rights_from_metadata.tn_access_restricted;
        if (parts.length == 2) {
          newsrc += '?' + parts[1];
        }
        
        $(img)
          .unbind("error")
          .attr("src", newsrc); 
      }
    };
    _replaceInaccessibleThumbnail(this);
  };
  $('.islandora-solr-search-result IMG')
    .on("error", replaceInaccessibleThumbnail);
})(jQuery);

