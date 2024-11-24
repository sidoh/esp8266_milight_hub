(function () {
  const cdnBase = "https://cdn.jsdelivr.net/gh/sidoh/esp8266_milight_hub@master/web2/dist/versions/{{tag}}/";
  const files = [
    {
      type: "stylesheet",
      cdnPath: cdnBase + "{{bundle_css:cdn_filename}}",
      localPath: "{{bundle_css:local_filename}}",
      size: parseInt("{{bundle_css:size}}", 10),
    },
    {
      type: "script",
      cdnPath: cdnBase + "{{bundle_js:cdn_filename}}",
      localPath: "{{bundle_js:local_filename}}",
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
        return Promise.resolve(response);
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
    const isDevelopment = "{{env}}" === "development";
    if (isDevelopment) {
      return tryLoadUrl(file.localPath, file.size, 0, 0);
    } else {
      return tryLoadUrl(file.cdnPath, file.size, 0, 0).catch(() =>
        tryLoadUrl(file.localPath, file.size, 0, 5)
      );
    }
  }

  function loadPage() {
    let currentFileIndex = 0;

    function loadNextFile() {
      if (currentFileIndex >= files.length) {
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
