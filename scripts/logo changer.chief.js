/*
 * Chief Client - Logo Changer
 * v1.0.0
 */

/**
 * @type {HTMLImageElement|void}
 */
let mainLogo;
/**
 * @type {string|void}
 */
let defaultLogo;

// set default
if (!localStorage.getItem("logo changer enabled"))
  localStorage.setItem("logo changer enabled", "off");

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations)
    for (const node of mutation.addedNodes) {
      if (node instanceof HTMLImageElement && node.id === 'mainLogo') {
        observer.disconnect();
        mainLogo = node;
        defaultLogo = node.src;
        updateLogo();
        return;
      }
    }
}).observe(document, {
  subtree: true,
  childList: true,
});

async function updateLogo() {
  if(!mainLogo) 
    alert("Logo Changer: Could not find logo element")
  
  const enabled = localStorage.getItem("logo changer enabled") === "on";

  if (enabled) mainLogo.src = localStorage.getItem("logo changer url");
  else mainLogo.src = defaultLogo;
}

exportUserscript({
  renderGameSettings: ({ html, UI, useLocalStorage }) => {
    const [localEnabled, setLocalEnabled] = useLocalStorage(
      "logo changer enabled"
    );

    const [logoURL, setLogoURL] = useLocalStorage("logo changer url");

    return html`
    <${UI.Set} title="Logo Changer">
      <${UI.Switch}
        title="Enabled"
        defaultChecked=${localEnabled === "on"}
        onChange=${(event) => {
          const newValue = event.currentTarget.checked ? "on" : "off";
          setLocalEnabled(newValue);
          updateLogo();
        }}
      />
      <${UI.FilePicker}
        title="Logo File"
        onChange=${(event) => {
          const [file] = event.currentTarget.files;
          if (!file) return;
          const fileReader = new FileReader();

          fileReader.addEventListener("load", () => {
            setLogoURL(fileReader.result);
            updateLogo();
          });

          fileReader.readAsDataURL(file);
        }}
      />
      <${UI.Text}
        title="Logo URL"
        value=${logoURL}
        onChange=${(event) => {
          setLogoURL(event.currentTarget.value);
          updateLogo();
        }}
      />
    </${UI.Set} />`;
  },
});
