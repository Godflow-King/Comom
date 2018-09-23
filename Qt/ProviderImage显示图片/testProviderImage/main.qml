import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.2

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Button{
        id:testbutton
        width:120
        height:120
        onClicked:
        {
            CodeImage.testShow();
        }
    }

    Image{
        id:img
        width:parent.width
        height:parent.height-testbutton.height
        anchors.top: testbutton.bottom
        cache: false
    }

    Connections{
        target: CodeImage
        onCallQmlRefeshImg:{
            img.source = "";
            img.source = "image://CodeImg";
        }
    }

//    MainForm {
//        anchors.fill: parent
//        mouseArea.onClicked: {
//            console.log(qsTr('Clicked on background. Text: "' + textEdit.text + '"'))
//        }
//        Image{
//            id:img
//            anchors.fill: parent
//        }

//        Connections{
//            target: CodeImage
//            onCallQmlRefeshImg:{
//                img.source = ""
//                img.source = "image://CodeImg"
//            }
//        }
//    }
}
