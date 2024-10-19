(function () {
  const cdnBase =
    "https://cdn.jsdelivr.net/gh/sidoh/esp8266_milight_hub@web2/";
  const files = [
    {
      type: "stylesheet",
      cdnPath: cdnBase + "{{bundle_css:filename}}",
      localPath: "{{bundle_css:filename_with_hash}}",
      size: parseInt("{{bundle_css:size}}", 10),
    },
    {
      type: "script",
      cdnPath: cdnBase + "{{bundle_js:filename}}",
      localPath: "{{bundle_js:filename_with_hash}}",
      size: parseInt("{{bundle_js:size}}", 10),
    },
  ];

  function createStylesheetLink(href) {
    const link = document.createElement("link");
    link.rel = "stylesheet";
    link.href = href;
    return link;
  }

  function createScript(src) {
    const script = document.createElement("script");
    script.src = src;
    script.defer = true;
    return script;
  }

  function tryLoadUrl(url, size, retryCount = 0, maxRetries = 5) {
    return fetch(url)
      .then((response) => {
        if (!response.ok) {
          throw new Error("Network response was not ok");
        }
        return response;
      })
      .catch((error) => {
        if (retryCount < maxRetries) {
          console.warn(`Retrying ${url} (${retryCount + 1}/${maxRetries})`);
          return tryLoadUrl(url, size, retryCount + 1, maxRetries);
        } else {
          throw error;
        }
      });
  }

  function tryLoadFile(file) {
    return tryLoadUrl(file.cdnPath, file.size, 0, 0)
      .then((response) => response.blob())
      .catch(() => {
        return tryLoadUrl(file.localPath, file.size);
      });
  }

  function loadPage() {
    let currentFileIndex = 0;

    function loadNextFile() {
      if (currentFileIndex >= files.length) {
        console.log("All files loaded");
        document.getElementById("loading").style.display = "none";
        return;
      }

      const file = files[currentFileIndex];

      tryLoadFile(file)
        .then((response) => {
          if (!response.ok) {
            throw new Error("Failed to load file");
          }
          return response.blob();
        })
        .then((blob) => {
          const url = URL.createObjectURL(blob);
          let element;

          if (file.type === "stylesheet") {
            element = createStylesheetLink(url);
          } else if (file.type === "script") {
            element = createScript(url);
          }

          document.head.appendChild(element);
          currentFileIndex++;
          loadNextFile();
        })
        .catch((error) => {
          console.error(`Failed to load file: ${file.cdnPath}`, error);
          currentFileIndex++;
          loadNextFile();
        });
    }

    // Start loading files
    loadNextFile();
  }

  document.addEventListener("DOMContentLoaded", () => {
    loadPage();
  });
})();
