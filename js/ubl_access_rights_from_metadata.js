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
      if (parts[0].endsWith('datastream/TN/view') || parts[0].endsWith('islandora/images/folder.png')) {
        var newsrc = tnurl;
        if (parts.length == 2) {
          newsrc += '?' + parts[1];
        }
        
        $(img)
          .unbind("error")
          .attr("src", newsrc)
          .parent().find(":hidden").show();
      }
    };
    _replaceInaccessibleThumbnail(this);
  };
  $(document).ready(function() {
    $('.islandora-solr-search-result IMG').on("error", replaceInaccessibleThumbnail);
    $('.islandora-object-thumb IMG').on("error", replaceInaccessibleThumbnail);
    $('.islandora-compound-thumbs .dc-grid-item IMG').on("error", replaceInaccessibleThumbnail);
  });
})(jQuery);

