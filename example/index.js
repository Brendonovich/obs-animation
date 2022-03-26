// @ts-check
const { OBS } = require("@sceneify/core");

async function main() {
  const obs = new OBS();

  await obs.connect("ws:localhost:4455");

  const targets = [11].map((i) => ({
    type: 0,
    sceneName: "_",
    sceneItemId: i,
    animations: [
      {
        property: "positionX",
        keyframes: [
          {
            timestamp: 0,
            value: 0,
          },
          {
            timestamp: 5000,
            value: 1720,
          },
        ],
      },
    ],
  }));

  await obs.call("CallVendorRequest", {
    vendorName: "obs-animation",
    requestType: "SetAnimation",
    requestData: {
      targets,
    },
  });
}

main();
