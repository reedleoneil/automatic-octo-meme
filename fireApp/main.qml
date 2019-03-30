import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtWebView 1.1
import QtMultimedia 5.8

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Fire App")
    SoundEffect {
        id: playSound
        source: "siren.wav"
    }
    Timer {
           interval: 1000; running: true; repeat: true
           onTriggered: {
               request('http://192.168.0.177', function (o) {

                                       // log the json response
                                       console.log(o.responseText);

                                       // translate response into object
                                       var d = eval('new Object(' + o.responseText + ')');

                                       // access elements inside json object with dot notation
                                       if(d.s1_triggered || d.s2_triggered || d.s3_triggered)
                                           playSound.play();
                                       else
                                           playSound.stop();

               });
           }
    }
    WebView {
            id: webView
            anchors.fill: parent
            url: path
            onLoadingChanged: {
                if (loadRequest.errorString)
                    console.error(loadRequest.errorString);
            }
    }
    function request(url, callback) {
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = (function(myxhr) {
                return function() {
                    callback(myxhr);
                }
            })(xhr);
            xhr.open('GET', url, true);
            xhr.send('');
    }
}
