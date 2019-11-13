// Helpers
    function handleError(error) {
        if (error) {
            console.error(error);
        }
    }

/* SOURCE EXAMPLE
    [23584:1028/110448.237:INFO:CONSOLE(67)] "{
    "id": "screen:0:0",
    "name": "Screen 1",
    "thumbnail": {},
    "display_id": "2528732444",
    "appIcon": null
    }"
*/

const imageWidth = 265;
const imageHeight = 165;

var images = 10;
var testImage = "resources/test.jpg";
function MakeSource(name, thumbnail, id, newWidth, newHeight){
    this.name = name;
    this.thumbnail = thumbnail;
    this.id = id;
    this.width = newWidth;
    this.height = newHeight;
}

let testSources = [];

for (let index = 0; index < images; index++) {
    let test = new MakeSource("REALLY LONG LONG title" + index, testImage, index, imageWidth, imageHeight);
    testSources.push(test);
}

const electron = require('electron');

let currentScreensharePickID = "";
function screensharePicked(id){
    currentScreensharePickID = id;
    document.getElementById("share_pick").innerHTML = "";
    addSource(sourceMap[id], "share_pick");
    togglePage();
}



function screenConfirmed(isConfirmed){
    if (isConfirmed === true){
        onAccessApproved(currentScreensharePickID);
    }
    togglePage();
}


let currentPage = "mainPage";
function togglePage(){
    if (currentPage === "mainPage") {
        currentPage = "confirmationPage";
        document.getElementById("select_screen").style.display = "none";
        document.getElementById("subtitle").innerHTML = "Confirm that you'd like to share this content.";
        document.getElementById("confirmation_screen").style.display = "block";
    } else {
        currentPage = "mainPage";
        document.getElementById("select_screen").style.display = "block";
        document.getElementById("subtitle").innerHTML = "Please select the content you'd like to share.";
        document.getElementById("confirmation_screen").style.display = "none";
    }
}

// UI
    function addSource(source, type) {
        let sourceBody = document.createElement('div')
        let thumbnail = source.thumbnail.toDataURL();
        sourceBody.classList.add("box")
        if (type === "share_pick") {
            sourceBody.style.marginLeft = "0px";
        }

        let image = "";
        if (source.appIcon) {
            image = `<img class="icon" src="${source.appIcon.toDataURL()}" />`;
        }
        sourceBody.innerHTML = `
            <div class="heading">
                ${image}
                <span class="screen_label">${source.name}</span>
            </div>
            <div class="${type === "share_pick" ? "image_no_hover" : "image"}" onclick="screensharePicked('${source.id}')">
                <img src="${thumbnail}" />
            </div>
        `
        if (type === "selects") {
            document.getElementById("selects").appendChild(sourceBody);
        } else {
            document.getElementById("share_pick").appendChild(sourceBody);
            document.getElementById("content_name").innerHTML = source.name;
        }
    }


    let sourceMap = {};
    function showSources() {
        document.getElementById("selects").innerHTML="";
        electron.desktopCapturer.getSources({ 
            types:['window', 'screen'],
            thumbnailSize: {
                width: imageWidth,
                height: imageHeight
            },
            fetchWindowIcons: true
        }, (error, sources) => {
            if (error) {
                console.log("Error getting sources", error);
            }

            // MN TODO:
            // Add all sources to array, sort array by type, then call `addSource()`
            // for all of those sources.

            for (let source of sources) {
                sourceMap[source.id] = source;
                    addSource(source, "selects");
            }
        });
    }


    let localStream;
    function stopSharing(){
        desktopSharing = false;

        if (localStream) {
            localStream.getTracks()[0].stop();
            localStream = null;
        }

        document.getElementById('screenshare').style.display = "none";
        stopTokBoxPublisher();
    }
  
    function gotStream(stream) {
        localStream = stream;
        startTokboxPublisher(localStream);

        stream.onended = () => {
            if (desktopSharing) {
                toggle();
            }
        };
    }


    function onAccessApproved(desktop_id) {
        if (!desktop_id) {
            console.log('Desktop Capture access rejected.');
            return;
        }
        showSources();
        document.getElementById('screenshare').style.visibility = "block";
        desktopSharing = true;
        navigator.webkitGetUserMedia({
            audio: false,
            video: {
                mandatory: {
                    chromeMediaSource: 'desktop',
                    chromeMediaSourceId: desktop_id,
                    minWidth: 1280,
                    maxWidth: 1280,
                    minHeight: 720,
                    maxHeight: 720
                }
            }
        }, gotStream, handleError);
    }

// Tokbox

    function initializeTokboxSession() {
        session = OT.initSession(projectAPIKey, sessionID);
        session.on('sessionDisconnected', (event) => {
            console.log('You were disconnected from the session.', event.reason);
        });

        // Connect to the session
        session.connect(token, (error) => {
            if (error) {
                handleError(error);
            }
        });
    }

  
    var publisher; 
    function startTokboxPublisher(stream){
        publisher = document.createElement("div");
        var publisherOptions = {
            videoSource: stream.getVideoTracks()[0],
            audioSource: null,
            insertMode: 'append',
            width: 1280,
            height: 720
        };

        publisher = OT.initPublisher(publisher, publisherOptions, function(error){
            if (error) {
                console.log("ERROR: " + error);
            } else {
                session.publish(publisher, function(error) {
                    if (error) {
                        console.log("ERROR FROM Session.publish: " + error);
                        return;
                    }
                    console.log("MADE IT TO PUBLISH")
                })
            }
        });
    }


    function stopTokBoxPublisher(){
        publisher.destroy();
    }

  
// main TODO:
    const ipcRenderer = electron.ipcRenderer;
    let projectAPIKey;
    let sessionID;
    let token;
    let session;

    ipcRenderer.on('connectionInfo', function(event, message){
        const connectionInfo = JSON.parse(message);
        projectAPIKey = connectionInfo.projectAPIKey;
        sessionID = connectionInfo.sessionID;
        token = connectionInfo.token;

        initializeTokboxSession();
    })


    document.addEventListener("DOMContentLoaded", () => {
        showSources();
    })
