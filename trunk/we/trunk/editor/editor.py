#!/usr/bin/env python

# This is only needed for Python v2 but is harmless for Python v3.
import sip
sip.setapi('QString', 2)

import math
import traceback

from PyQt4 import QtCore, QtGui, QtXml

import editor_rc

class scene_visitor:
    def __init__(self):
        pass

    def visit_scene(self, scene):
        print >>sys.stderr, "visiting scene: with %d items" % (len(scene.items()))
        for item in scene.items(): item.accept(self)

    def visit_edge(self, edge):
        print >>sys.stderr, "link between %s and %s" % (edge.startItem(), edge.endItem())
        edge.startItem().accept(self)
        edge.endItem().accept(self)

    def visit_place(self, p):
        print >>sys.stderr, "place:", p

    def visit_transition(self, t):
        print >>sys.stderr, "transition:", t

class xml_visitor(scene_visitor):
    def __init__(self):
        scene_visitor.__init__(self)
        self.doc = QtXml.QDomDocument()

        self._visited = {}

    def visited(self, thing):
        return thing in self._visited
    def id_of(self, thing):
        return self._visited[thing]
    def mark_visited(self, thing):
        id = len(self._visited)
        self._visited[thing] = id
        return id

    def visit_scene(self, scene):
        self.root = self.doc.createElement("workflow")
        self.doc.appendChild(self.root)

        for item in scene.items(): item.accept(self)

    def visit_edge(self, edge):
        print >>sys.stderr, "link between %s and %s" % (edge.startItem(), edge.endItem())
        edge.startItem().accept(self)
        edge.endItem().accept(self)

        src = self.id_of(edge.startItem())
        dst = self.id_of(edge.endItem())
        e = self.doc.createElement("edge")
        e.setAttribute("from", src)
        e.setAttribute("to", dst)
        self.root.appendChild(e)

    def visit_place(self, p):
        print >>sys.stderr, "place:", p
        # make sure that we visit places only once
        if self.visited(p): return

        id = self.mark_visited(p)

        e = self.doc.createElement("place")
        e.setAttribute("name", id)
        e.setAttribute("pos", "(%s, %s)" % (p.pos().x(), p.pos().y()))
        self.root.appendChild(e)

    def visit_transition(self, t):
        print >>sys.stderr, "trans:", t
        # make sure that we visit transitions only once
        if self.visited(t): return

        id = self.mark_visited(t)

        e = self.doc.createElement("transition")
        e.setAttribute("name", id)
        e.setAttribute("pos", "(%s, %s)" % (t.pos().x(), t.pos().y()))
        self.root.appendChild(e)

class Edge(QtGui.QGraphicsLineItem):
    def __init__(self, startItem, endItem, parent=None, scene=None):
        super(Edge, self).__init__(parent, scene)

        self.edgeHead = QtGui.QPolygonF()

        self.myStartItem = startItem
        self.myEndItem = endItem
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
        self.myColor = QtCore.Qt.black
        self.setPen(QtGui.QPen(self.myColor, 1, QtCore.Qt.SolidLine,
                QtCore.Qt.RoundCap, QtCore.Qt.RoundJoin))

    def accept(self, visitor):
        visitor.visit_edge(self)

    def setColor(self, color):
        self.myColor = color

    def startItem(self):
        return self.myStartItem

    def endItem(self):
        return self.myEndItem

    def boundingRect(self):
        extra = (self.pen().width() + 20) / 2.0
        p1 = self.line().p1()
        p2 = self.line().p2()
        return QtCore.QRectF(p1, QtCore.QSizeF(p2.x() - p1.x(), p2.y() - p1.y())).normalized().adjusted(-extra, -extra, extra, extra)

    def shape(self):
        path = super(Edge, self).shape()
        path.addPolygon(self.edgeHead)
        return path

    def updatePosition(self):
        line = QtCore.QLineF(self.mapFromItem(self.myStartItem, 0, 0), self.mapFromItem(self.myEndItem, 0, 0))
        self.setLine(line)

    def paint(self, painter, option, widget=None):
        if (self.myStartItem.collidesWithItem(self.myEndItem)):
            return

        myStartItem = self.myStartItem
        myEndItem = self.myEndItem
        myColor = self.myColor
        myPen = self.pen()
        myPen.setColor(self.myColor)
        arrowSize = 7.0
        painter.setPen(myPen)
        painter.setBrush(self.myColor)

        centerLine = QtCore.QLineF(myStartItem.pos(), myEndItem.pos())
        endPolygon = myEndItem.polygon()
        p1 = endPolygon.first() + myEndItem.pos()

        intersectPoint = QtCore.QPointF()
        for i in endPolygon:
            p2 = i + myEndItem.pos()
            polyLine = QtCore.QLineF(p1, p2)
            intersectType = polyLine.intersect(centerLine, intersectPoint)
            if intersectType == QtCore.QLineF.BoundedIntersection:
                break
            p1 = p2

        self.setLine(QtCore.QLineF(intersectPoint, myStartItem.pos()))
        line = self.line()

        angle = math.acos(line.dx() / line.length())
        if line.dy() >= 0:
            angle = (math.pi * 2.0) - angle

        arrowP1 = line.p1() + QtCore.QPointF(math.sin(angle + math.pi / 3.0) * arrowSize,
                                        math.cos(angle + math.pi / 3) * arrowSize)
        arrowP2 = line.p1() + QtCore.QPointF(math.sin(angle + math.pi - math.pi / 3.0) * arrowSize,
                                        math.cos(angle + math.pi - math.pi / 3.0) * arrowSize)

        self.edgeHead.clear()
        for point in [line.p1(), arrowP1, arrowP2]:
            self.edgeHead.append(point)

        painter.drawLine(line)
        painter.drawPolygon(self.edgeHead)
        if self.isSelected():
            painter.setPen(QtGui.QPen(myColor, 1, QtCore.Qt.DashLine))
            myLine = QtCore.QLineF(line)
            myLine.translate(0, 4.0)
            painter.drawLine(myLine)
            myLine.translate(0,-8.0)
            painter.drawLine(myLine)

class DiagramItem(QtGui.QGraphicsPolygonItem):
    Place, Transition = range(2)

    def __init__(self, diagramType, contextMenu, parent=None, scene=None):
        super(DiagramItem, self).__init__(parent, scene)

        self.edges = []

        self.diagramType = diagramType
        self.contextMenu = contextMenu
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)

    def accept(self, visitor):
        if self.diagramType == self.Place: visitor.visit_place(self)
        if self.diagramType == self.Transition: visitor.visit_transition(self)

    def removeEdge(self, edge):
        try:
            self.edges.remove(edge)
        except ValueError:
            pass

    def removeArrows(self):
        for edge in self.edges[:]:
            edge.startItem().removeEdge(edge)
            edge.endItem().removeEdge(edge)
            self.scene().removeItem(edge)

    def addEdge(self, edge):
        self.edges.append(edge)

    def image(self):
      raise "not implemented"

    def contextMenuEvent(self, event):
        self.scene().clearSelection()
        self.setSelected(True)
        self.myContextMenu.exec_(event.screenPos())

    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemPositionChange:
            for edge in self.edges:
                edge.updatePosition()
        return value

    def canConnectFrom(self, src):
      return True
    def canConnectTo(self, dst):
      return True

class Place(DiagramItem):
  def __init__(self, contextMenu, parent=None, scene=None):
    DiagramItem.__init__(self, DiagramItem.Place, contextMenu, parent, scene)
    path = QtGui.QPainterPath()
    path.addEllipse(-15, -15, 30, 30)
    self.setPolygon(path.toFillPolygon())

  def __str__(self):
    return "place (%s)" % (DiagramItem.__str__(self))

  def canConnectFrom(self, src):
    return isinstance(src, Transition)

  def canConnectTo(self, dst):
    return isinstance(dst, Transition)

  def image(self):
    pixmap = QtGui.QPixmap(250, 250)
    pixmap.fill(QtCore.Qt.transparent)
    painter = QtGui.QPainter(pixmap)
    painter.setPen(QtGui.QPen(QtCore.Qt.black, 8))
    painter.translate(125, 125)

    path = QtGui.QPainterPath()
    path.addEllipse(-70, -70, 140, 140)
    painter.drawPolyline(path.toFillPolygon())
    return pixmap

class Transition(DiagramItem):
  def __init__(self, contextMenu, parent=None, scene=None):
    DiagramItem.__init__(self, DiagramItem.Transition, contextMenu, parent, scene)
    path = QtGui.QPainterPath()
    path.addRect(-25, -10, 50, 20)
    self.setPolygon(path.toFillPolygon())

  def __str__(self):
    return "transition (%s)" % (DiagramItem.__str__(self))

  def canConnectFrom(self, src):
    return isinstance(src, Place)

  def canConnectTo(self, dst):
    return isinstance(dst, Place)

  def image(self):
    pixmap = QtGui.QPixmap(250, 250)
    pixmap.fill(QtCore.Qt.transparent)
    painter = QtGui.QPainter(pixmap)
    painter.setPen(QtGui.QPen(QtCore.Qt.black, 8))
    painter.translate(125, 125)

    path = QtGui.QPainterPath()
    path.addRect(-100, -50, 200, 100)
    painter.drawPolyline(path.toFillPolygon())
    return pixmap

class DiagramScene(QtGui.QGraphicsScene):
    InsertItem, InsertLine, InsertText, MoveItem  = range(4)

    itemInserted = QtCore.pyqtSignal(DiagramItem)

    textInserted = QtCore.pyqtSignal(QtGui.QGraphicsTextItem)

    itemSelected = QtCore.pyqtSignal(QtGui.QGraphicsItem)

    def __init__(self, itemMenu, parent=None):
        super(DiagramScene, self).__init__(parent)

        self.myItemMenu = itemMenu
        self.myMode = self.MoveItem
        self.myItemType = DiagramItem.Place
        self.line = None
        self.textItem = None
        self.myItemColor = QtCore.Qt.white
        self.myTextColor = QtCore.Qt.black
        self.myLineColor = QtCore.Qt.black

    def accept(self, visitor):
        visitor.visit_scene(self)

    def setLineColor(self, color):
        self.myLineColor = color
        if self.isItemChange(Arrow):
            item = self.selectedItems()[0]
            item.setColor(self.myLineColor)
            self.update()

    def setTextColor(self, color):
        self.myTextColor = color
        if self.isItemChange(DiagramTextItem):
            item = self.selectedItems()[0]
            item.setDefaultTextColor(self.myTextColor)

    def setItemColor(self, color):
        self.myItemColor = color
        if self.isItemChange(DiagramItem):
            item = self.selectedItems()[0]
            item.setBrush(self.myItemColor)

    def setMode(self, mode):
        self.myMode = mode

    def setItemType(self, type):
        self.myItemType = type

    def editorLostFocus(self, item):
        cursor = item.textCursor()
        cursor.clearSelection()
        item.setTextCursor(cursor)

        if item.toPlainText():
            self.removeItem(item)
            item.deleteLater()

    def addItemByType(self, type, pos):
      if type == DiagramItem.Place:
        item = Place(self.myItemMenu)
      elif type == DiagramItem.Transition:
        item = Transition(self.myItemMenu)
      else:
        raise Exception("unknown item type %s" % (type))
      item.setBrush(self.myItemColor)
      self.addItem(item)
      if pos is not None:
        item.setPos(pos)
      self.itemInserted.emit(item)
      return item

    def addPlace(self, pos=None):
        return self.addItemByType(DiagramItem.Place, pos)

    def addTransition(self, pos=None):
        return self.addItemByType(DiagramItem.Transition, pos)

    def mousePressEvent(self, mouseEvent):
        if (mouseEvent.button() != QtCore.Qt.LeftButton):
            return

        if self.myMode == self.InsertItem:
            self.addItemByType(self.myItemType, mouseEvent.scenePos())
        elif self.myMode == self.InsertLine:
            self.line = QtGui.QGraphicsLineItem(QtCore.QLineF(mouseEvent.scenePos(),
                                        mouseEvent.scenePos()))
            self.line.setPen(QtGui.QPen(self.myLineColor, 2))
            self.addItem(self.line)

        super(DiagramScene, self).mousePressEvent(mouseEvent)

    def mouseMoveEvent(self, mouseEvent):
        if self.myMode == self.InsertLine and self.line:
            newLine = QtCore.QLineF(self.line.line().p1(), mouseEvent.scenePos())
            self.line.setLine(newLine)
        elif self.myMode == self.MoveItem:
            super(DiagramScene, self).mouseMoveEvent(mouseEvent)

    def mouseReleaseEvent(self, mouseEvent):
        if self.line and self.myMode == self.InsertLine:

            startItems = self.items(self.line.line().p1())
            if len(startItems): startItems.remove(self.line)

            endItems = self.items(self.line.line().p2())
            if len(endItems): endItems.remove(self.line)

            self.removeItem(self.line)
            self.line = None

            if len(startItems) and len(endItems):
              src = startItems[0]
              dst = endItems[0]
              if dst.canConnectFrom(src) and src.canConnectTo(dst):
                self.connectItems(src, dst)
              else:
                # TODO: should go to status bar
                print >>sys.stderr, "Cannot connect: %s -> %s" % (src, dst)

        self.line = None
        super(DiagramScene, self).mouseReleaseEvent(mouseEvent)

    def connectItems(self, src, dst):
        edge = Edge(src, dst)
        edge.setColor(self.myLineColor)
        src.addEdge(edge)
        dst.addEdge(edge)
        edge.setZValue(-1000.0)
        self.addItem(edge)
        edge.updatePosition()
        return edge

    def isItemChange(self, type):
        for item in self.selectedItems():
            if isinstance(item, type):
                return True
        return False

class DragItem(QtGui.QToolButton):
  def __init__(self, type, parent=None):
    QtGui.QToolButton.__init__(self, parent)
    if type == DiagramItem.Place:
      self.image = Place(None).image()
    elif type == DiagramItem.Transition:
      self.image = Transition(None).image()
    self.type = type
    self.setIcon(QtGui.QIcon(self.image))
    self.setIconSize(QtCore.QSize(50, 50))
    self.setCheckable(True)

  def mousePressEvent(self, event):
    #QtGui.QToolButton.mousePressEvent(self, event)
    pass

  def mouseMoveEvent(self, event):
    if event.buttons() != QtCore.Qt.LeftButton:
      return

    itemData = QtCore.QByteArray()
    dataStream = QtCore.QDataStream(itemData, QtCore.QIODevice.WriteOnly)
    dataStream.writeInt32(int(self.type))

    mimeData = QtCore.QMimeData()
    mimeData.setData("application/x-pnet-item", itemData)

    drag = QtGui.QDrag(self)
    drag.setMimeData(mimeData)
    drag.setPixmap(self.icon().pixmap(50))
    drag.setHotSpot(event.pos() - self.pos())

    dropAction = drag.start(QtCore.Qt.MoveAction)

    if dropAction == QtCore.Qt.MoveAction:
        self.close()

class EditorView(QtGui.QGraphicsView):
  def __init__(self, scene, parent=None):
    QtGui.QGraphicsView.__init__(self, scene, parent)
    self.setAcceptDrops(True)

  def dragEnterEvent(self, event):
    if event.mimeData().hasFormat("application/x-pnet-item"):
      if event.source() == self:
        event.setDropAction(QtCore.Qt.MoveAction)
        event.accept()
      else:
        event.acceptProposedAction()
    else:
      print "ignoring invalid mimedata: %s" % (event.mimeData())

  dragMoveEvent = dragEnterEvent

  def dropEvent(self, event):
    if event.mimeData().hasFormat("application/x-pnet-item"):
      itemData = event.mimeData().data("application/x-pnet-item")
      dataStream = QtCore.QDataStream(itemData, QtCore.QIODevice.ReadOnly)
      type = int(dataStream.readInt32())
      if type == DiagramItem.Place:
        self.scene().addPlace(self.mapToScene(event.pos()))
      if type == DiagramItem.Transition:
        self.scene().addTransition(self.mapToScene(event.pos()))
      else:
        event.ignore()

      if event.source() == self:
        event.setDropAction(QtCore.Qt.QMoveAction)
        event.accept()
      else:
        event.acceptProposedAction()
    else:
      event.ignore()

class MainWindow(QtGui.QMainWindow):
    InsertTextButton = 10

    def __init__(self):
        super(MainWindow, self).__init__()

        self.createActions()
        self.createMenus()
        self.createToolBox()

        self.scene = DiagramScene(self.itemMenu)
        self.scene.setSceneRect(QtCore.QRectF(0, 0, 5000, 5000))
        self.scene.itemInserted.connect(self.itemInserted)
        self.scene.itemSelected.connect(self.itemSelected)

        self.createToolbars()

        layout = QtGui.QHBoxLayout()
        layout.addWidget(self.toolBox)
        self.tabWidget = QtGui.QTabWidget()
        self.view = EditorView(self.scene)
        self.tabWidget.addTab(self.view, "untitled")
        self.tabWidget.setTabsClosable (True)
        self.tabWidget.tabCloseRequested.connect(self.tabClose)
        layout.addWidget(self.tabWidget)

        self.widget = QtGui.QWidget()
        self.widget.setLayout(layout)
        self.setCentralWidget(self.widget)
        self.setWindowTitle("Petri-Net Editor")

        self.statusBar().showMessage("Petri-Net Editor initialized", 2000)
        if len(sys.argv) > 1:
          self.loadFile(sys.argv[1])
        else:
          self.setCurrentFile(None)

    def tabClose(self, tab):
      # TODO check for changes in tab
      # TODO scene must be attached to tab widget, not so self
      print >>sys.stderr, "tab should be closed"
      #self.tabWidget.removeTab(tab)

    def backgroundButtonGroupClicked(self, button):
        buttons = self.backgroundButtonGroup.buttons()
        for myButton in buttons:
            if myButton != button:
                button.setChecked(False)

        self.scene.update()
        self.view.update()

    def buttonGroupClicked(self, id):
        buttons = self.buttonGroup.buttons()
        for button in buttons:
            if self.buttonGroup.button(id) != button:
                button.setChecked(False)

        if id == self.InsertTextButton:
            self.scene.setMode(DiagramScene.InsertText)
        else:
            self.scene.setItemType(id)
            self.scene.setMode(DiagramScene.InsertItem)

    def deleteItem(self):
        for item in self.scene.selectedItems():
            if isinstance(item, DiagramItem):
                item.removeArrows()
            self.scene.removeItem(item)

    def pointerGroupClicked(self, i):
        self.scene.setMode(self.pointerTypeGroup.checkedId())

    def itemInserted(self, item):
        self.pointerTypeGroup.button(DiagramScene.MoveItem).setChecked(True)
        self.scene.setMode(self.pointerTypeGroup.checkedId())
        self.buttonGroup.button(item.diagramType).setChecked(False)

    def sceneScaleChanged(self, scale):
        newScale = float( scale[:-1] ) / 100.0
        oldMatrix = self.view.matrix()
        self.view.resetMatrix()
        self.view.translate(oldMatrix.dx(), oldMatrix.dy())
        self.view.scale(newScale, newScale)

    def lineButtonTriggered(self):
        self.scene.setLineColor(QtGui.QColor(self.lineAction.data()))

    def itemSelected(self, item):
      pass

    def about(self):
        QtGui.QMessageBox.about(self, "About Diagram Scene",
                "The <b>Diagram Scene</b> example shows use of the graphics framework.")

    def createToolBox(self):
        self.buttonGroup = QtGui.QButtonGroup()
        self.buttonGroup.setExclusive(False)
        self.buttonGroup.buttonClicked[int].connect(self.buttonGroupClicked)

        layout = QtGui.QGridLayout()
        layout.addWidget(self.createCellWidget("Place", DiagramItem.Place),
                0, 0)
        layout.addWidget(self.createCellWidget("Transition", DiagramItem.Transition), 0,
                1)

        layout.setRowStretch(3, 10)
        layout.setColumnStretch(2, 10)

        itemWidget = QtGui.QWidget()
        itemWidget.setLayout(layout)

        self.backgroundButtonGroup = QtGui.QButtonGroup()
        self.backgroundButtonGroup.buttonClicked.connect(self.backgroundButtonGroupClicked)

        backgroundLayout = QtGui.QGridLayout()
        backgroundLayout.setRowStretch(2, 10)
        backgroundLayout.setColumnStretch(2, 10)

        backgroundWidget = QtGui.QWidget()
        backgroundWidget.setLayout(backgroundLayout)

        self.toolBox = QtGui.QToolBox()
        self.toolBox.setSizePolicy(QtGui.QSizePolicy(QtGui.QSizePolicy.Maximum, QtGui.QSizePolicy.Ignored))
        self.toolBox.setMinimumWidth(itemWidget.sizeHint().width())
        self.toolBox.addItem(itemWidget, "Basic Petri-Net Items")
        self.toolBox.addItem(backgroundWidget, "Library")

    def createActions(self):
        self.deleteAction = QtGui.QAction(QtGui.QIcon(':/images/delete.svg'),
                "&Delete", self, shortcut="Delete",
                statusTip="Delete item from diagram",
                triggered=self.deleteItem)

        self.exitAction = QtGui.QAction(
            QtGui.QIcon(":/images/exit.svg"), "&Exit", self, shortcut="Ctrl+W",
            statusTip="Exit the Petri-Net editor", triggered=self.close)

        self.aboutAction = QtGui.QAction("A&bout", self, shortcut="Ctrl+B",
                triggered=self.about)

        self.openAction = QtGui.QAction(
            QtGui.QIcon(":/images/open.svg"), "&Open", self, shortcut="Ctrl+O", triggered=self.open, statusTip="Open network")
        self.saveAction = QtGui.QAction(
            QtGui.QIcon(":/images/save.svg"), "&Save", self, shortcut="Ctrl+S", triggered=self.save, statusTip="Save network")
        self.saveAsAction = QtGui.QAction(
            QtGui.QIcon(":/images/save-as.svg"), "Save &As...", self, triggered=self.saveAs, statusTip="Save network in a different file")
#        self.saveAction.setEnabled( False )

    def createMenus(self):
        self.fileMenu = self.menuBar().addMenu("&File")
        self.fileMenu.addAction(self.openAction)
        self.fileMenu.addAction(self.saveAction)
        self.fileMenu.addAction(self.saveAsAction)
        self.fileMenu.addSeparator()
        self.fileMenu.addAction(self.exitAction)

        self.itemMenu = self.menuBar().addMenu("&Item")
        self.itemMenu.addAction(self.deleteAction)

        self.aboutMenu = self.menuBar().addMenu("&Help")
        self.aboutMenu.addAction(self.aboutAction)

    def createToolbars(self):
        self.fileToolBar = self.addToolBar("File")
        self.fileToolBar.addAction(self.openAction)
        self.fileToolBar.addAction(self.saveAction)
        self.fileToolBar.addAction(self.saveAsAction)

        self.editToolBar = self.addToolBar("Edit")
        self.editToolBar.addAction(self.deleteAction)

        pointerButton = QtGui.QToolButton()
        pointerButton.setCheckable(True)
        pointerButton.setChecked(True)
        pointerButton.setIcon(QtGui.QIcon(':/images/pointer.png'))
        linePointerButton = QtGui.QToolButton()
        linePointerButton.setCheckable(True)
        linePointerButton.setIcon(QtGui.QIcon(':/images/edge.svg'))

        self.pointerTypeGroup = QtGui.QButtonGroup()
        self.pointerTypeGroup.addButton(pointerButton, DiagramScene.MoveItem)
        self.pointerTypeGroup.addButton(linePointerButton,
                DiagramScene.InsertLine)
        self.pointerTypeGroup.buttonClicked[int].connect(self.pointerGroupClicked)

        self.sceneScaleCombo = QtGui.QComboBox()
        self.sceneScaleCombo.addItems(["50%", "75%", "100%", "125%", "150%"])
        self.sceneScaleCombo.setCurrentIndex(2)
        self.sceneScaleCombo.currentIndexChanged[str].connect(self.sceneScaleChanged)

        self.pointerToolbar = self.addToolBar("Pointer type")
        self.pointerToolbar.addWidget(pointerButton)
        self.pointerToolbar.addWidget(linePointerButton)
        self.pointerToolbar.addWidget(self.sceneScaleCombo)

    def createCellWidget(self, text, diagramType):
        button = DragItem(diagramType)
        self.buttonGroup.addButton(button, diagramType)

        layout = QtGui.QGridLayout()
        layout.addWidget(button, 0, 0, QtCore.Qt.AlignHCenter)
        layout.addWidget(QtGui.QLabel(text), 1, 0, QtCore.Qt.AlignCenter)

        widget = QtGui.QWidget()
        widget.setLayout(layout)

        return widget

    def open(self):
        # TODO: this should actually check for unsaved modifications
        if len(self.scene.items()) > 0:
            answer = QtGui.QMessageBox.question(self, "Clear current net?", "Your net contains elements, i have to remove them!", QtGui.QMessageBox.Ok | QtGui.QMessageBox.Cancel)
            if answer == QtGui.QMessageBox.Ok:
                self.scene.clear()
            else:
                return
        fileName = QtGui.QFileDialog.getOpenFileName(self)
        if fileName:
            self.loadFile(fileName)

    def save(self):
        if self.current_file:
            return self.saveFile(self.current_file)
        else:
            return self.saveAs()

    def saveAs(self):
        fileName = QtGui.QFileDialog.getSaveFileName(self)
        if fileName:
            return self.saveFile(fileName)

    def saveFile(self, fileName):
        file = QtCore.QFile(fileName)
        if not file.open(QtCore.QFile.WriteOnly | QtCore.QFile.Text):
            QtGui.QMessageBox.critical(self, "Petri-Net Editor",
                    "Cannot write file %s:\n%s." % (fileName, file.errorString()))
            return False
        outf = QtCore.QTextStream(file)
        try:
            QtGui.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
            debug_visitor = scene_visitor()
            self.scene.accept(debug_visitor)
            visitor = xml_visitor()
            self.scene.accept(visitor)

            outf << visitor.doc
            self.debugXml(visitor.doc)
            self.statusBar().showMessage("Network saved to file: %s" % (fileName), 2000)
            self.setCurrentFile(fileName);
        except:
            QtGui.QMessageBox.critical(self, "Error", "File could not be saved:\n\n%s." % (traceback.format_exc()))
            return False
        finally:
            QtGui.QApplication.restoreOverrideCursor()

        return True

    def loadFile(self, fileName):
        file = QtCore.QFile(fileName)
        if not file.open(QtCore.QFile.ReadOnly | QtCore.QFile.Text):
            QtGui.QMessageBox.warning(self, "Petri-Net Editor",
                    "Cannot open file %s:\n%s." % (fileName, file.errorString()))
            return False
        try:
            QtGui.QApplication.setOverrideCursor(QtCore.Qt.WaitCursor)
            doc = QtXml.QDomDocument()
            doc.setContent(file)
            self.debugXml(doc)

            # collect all things
            things = {}
            workflow_node = doc.documentElement()
            thing = workflow_node.firstChild()
            while not thing.isNull():
                if not thing.isElement(): continue
                e = thing.toElement()
                if e.tagName() == "place":
                  id = e.attribute("name")
                  x,y = eval( e.attribute("pos", "(0,0)"))
                  p = self.scene.addPlace( QtCore.QPointF(x,y) )
                  things[id] = p
                if e.tagName() == "transition":
                  id = e.attribute("name")
                  x,y = eval( e.attribute("pos", "(0,0)"))
                  t = self.scene.addTransition( QtCore.QPointF(x,y) )
                  things[id] = t
                if e.tagName() == "edge":
                  src = things[e.attribute("from")]
                  dst = things[e.attribute("to")]
                  self.scene.connectItems(src, dst)
                thing = thing.nextSibling()

            self.statusBar().showMessage("Data loaded from file: %s" % fileName, 2000)
            self.setCurrentFile(fileName)
        finally:
            QtGui.QApplication.restoreOverrideCursor()
        return True

    def setCurrentFile(self, filename):
        self.current_file = filename
        if self.current_file:
            shownName = self.strippedName(self.current_file)
        else:
            shownName = 'untitled'
        self.setWindowTitle("%s[*] - Petri-Net Editor" % shownName)

    def strippedName(self, filename):
        return QtCore.QFileInfo(filename).fileName()

    def debugXml(self, doc):
        buf = QtCore.QBuffer()
        buf.open(QtCore.QBuffer.ReadWrite);
        text = QtCore.QTextStream(buf)
        text << doc
        sys.stderr.write(buf.buffer())

if __name__ == '__main__':

    import sys

    app = QtGui.QApplication(sys.argv)

    mainWindow = MainWindow()
    mainWindow.setGeometry(100, 100, 800, 500)
    mainWindow.show()

    sys.exit(app.exec_())
