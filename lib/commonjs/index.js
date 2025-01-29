"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
var _TensorflowLite = require("./TensorflowLite");
Object.keys(_TensorflowLite).forEach(function (key) {
  if (key === "default" || key === "__esModule") return;
  if (key in exports && exports[key] === _TensorflowLite[key]) return;
  Object.defineProperty(exports, key, {
    enumerable: true,
    get: function () {
      return _TensorflowLite[key];
    }
  });
});
//# sourceMappingURL=index.js.map