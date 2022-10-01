"use strict";

import Tab from "./Tab";
import Window from "./Window";
import utils from "../Utils";

export default class TabWindow extends Window {
  tabs = new Set();
  constructor(menu) {
    super(menu);

    this.tab_layout = utils.add_ele("div", this.header, {
      style: {
        position: "relative",
        "z-index": 9,
      },
    });
  }
  tab(label) {
    const tab = new Tab(this, label);

    this.tabs.add(tab);

    return tab;
  }
  get main_tab() {
    let first;

    for (const tab of this.tabs) {
      first = first || tab;
      if (tab.visible) return tab;
    }

    return first;
  }
  update(init) {
    for (const tab of this.tabs) {
      tab.update(init);
      if (tab != this.main_tab) tab.hide();
    }

    this.main_tab.show();
  }
}
