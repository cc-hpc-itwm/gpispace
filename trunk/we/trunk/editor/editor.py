#!/usr/bin/env python

import sys
from PyQt4 import QtCore, QtGui, QtXml

class Editor(QtGui.QMainWindow):
    def __init__(self):
        super(Editor, self).__init__()

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)

    mainWindow = Editor()
    mainWindow.setGeometry(100, 100, 800, 500)
    mainWindow.show()

    sys.exit(app.exec_())
