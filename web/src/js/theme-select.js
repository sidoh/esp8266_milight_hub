// Change current theme
// Adapted from : https://wdtz.org/bootswatch-theme-selector.html

$.getJSON("https://bootswatch.com/api/3.json", function (data) {
  var themes = data.themes;
  var list = $("#theme-list");
  themes.forEach(function(value, index){
    var elmt = '<li><a href="#" class="change-style-menu-item" url='+value.cssCdn+'>'+value.name+'</a></li>'
    list.append(elmt);
  });
});

var supports_storage = supports_html5_storage();

if (supports_storage) {
  var theme = localStorage.theme;
  if ( typeof theme != 'undefined' ) {
    console.log("Changing theme to " + theme);
    $('link[title="main"]').attr('href', localStorage.theme_url)
    changeHeight()
  }
} 

// New theme selected
jQuery(function($){
  $('body').on('click', '.change-style-menu-item', function() {
    var theme_name = $(this).html();
    $('#theme-label').html("Theme : " + theme_name);
    console.log("Theme set to " + theme_name);
    var url_theme = "";
    if ( theme_name === "Bootstrap (Default)" ) {
      url_theme = "https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css";
    } else {
      url_theme = $(this).attr('url');
    }
    if (supports_storage) {
      // Save into the local database the selected theme
      localStorage.theme = theme_name;
      localStorage.theme_url = url_theme;
    }
    console.log("URL theme : " + url_theme);
    $('link[title="main"]').attr('href', url_theme);
    changeHeight()
  });
});

// Local storage available ?
function supports_html5_storage(){
  try {
    return 'localStorage' in window && window['localStorage'] !== null;
  } catch (e) {
    return false;
  }
}  

function changeHeight() {
  setTimeout(function() {
    var btnCssHeight = $("#btnHeight").css("height");
    var raw = btnCssHeight.replace("px", "")
    $(".plus-minus-group .title").css("height", btnCssHeight);
    $('input[name="status"]')
      .bootstrapToggle('destroy')
      .bootstrapToggle({
        height: raw
      });
  }, 1000)
}