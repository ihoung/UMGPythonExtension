from Dependencies.scripting_widget import singleton, widgetXML

@singleton
@widgetXML('/Game/Python/ExampleWidget/ExampleWidget.xml')
class ExampleWidget:
    def __init__(self):
        # Add any binding action here
        # e.g. 
        # self.ui.Button.on_clicked.add_callable(lambda:print("button on clicked"))
        pass

    def show(self):
        pass

    def close(self):
        pass


def main():
    a=ExampleWidget()
    a.show()
