/**
 * @file
 * js/ubl_details_tools.js
 */

jQuery(document).ready(function() {
  var $additionalblock = jQuery('DIV.download-additional'); 
  if (jQuery('.dc-box .dc-metadata .islandora-metadata-fields').size() > 0) {
    var $metadataTable = jQuery('.dc-box .dc-metadata .islandora-metadata-fields');
    var accesstitle = $additionalblock.find('.accesstitle').html();
    var accesscontent = $additionalblock.find('.accesscontent').html();
    var usetitle = $additionalblock.find('.usetitle').html();
    var usecontent = $additionalblock.find('.usecontent').html();

    if (typeof accesstitle === 'string' && accesstitle.length > 0) {
      $metadataTable.find('TBODY').append('<TR><TH>' + accesstitle + '</TH><TD>' + accesscontent + '</TD></TR>'); 
    }
    if (typeof usetitle === 'string' && usetitle.length > 0) {
      $metadataTable.find('TBODY').append('<TR><TH>' + usetitle + '</TH><TD>' + usecontent + '</TD></TR>'); 
    }
  } 
  jQuery('.ubl-detail-tools LI A .fa-download').parent().click(function (e) {
    var $li = jQuery(this).parent();
    if ($additionalblock.size() > 0) {
      e.preventDefault();
      if ($additionalblock.is(':visible')) {
        return; // is already visible
      }
      $additionalblock.find('TR').each(function() {
        var downloadurl = jQuery(this).data('download');
        jQuery(this).click(function (e) {
          if (downloadurl) {
            document.location = downloadurl;
          }
        });
      });
      var x = $li.offset().left - $additionalblock.outerWidth() + $li.outerWidth();
      if (x > 0) {
        $additionalblock.css('left', x + 'px');
      }
      var y = $li.offset().top + $li.outerHeight();
      $additionalblock.css('top', y + 'px');

      $additionalblock.slideDown();
      e.stopPropagation();
      jQuery('BODY').one('click', function() {
        $additionalblock.slideUp();
      });
    }
  });
});
