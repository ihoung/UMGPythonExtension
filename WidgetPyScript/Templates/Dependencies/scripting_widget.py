import os
import glob
import functools
import xml.etree.ElementTree as ET
import re
import unreal

def convert_to_absolute_path(path:str):
    path_parts = path.split('/')
    if len(path_parts) > 1:
        if path_parts[1] == 'Game':
            proj_content_dir = unreal.Paths.project_content_dir()
            return path.replace('/Game/', proj_content_dir, 1)
        else:
            plugin_name = path_parts[1]
            matched_plugins = glob.glob(unreal.Paths.project_plugins_dir()+'**/{}.uplugin'.format(plugin_name))
            if len(matched_plugins) > 0:
                plugin_dir = os.path.dirname(matched_plugins[0])
                return path.replace('/'+plugin_name, plugin_dir, 1)
    return path

def get_xml_root(xml_path:str):
    with open(xml_path) as file:
        xml_text = file.read()
    try:
        root = ET.fromstring(re.sub(r"(<\?xml[^>]+\?>)", r"\1<root>", xml_text) + "</root>")
        return root
    except ET.ParseError as err:
        print('Invalid XML file. Please check the text format.\nUnexpected {} : {}'.format(type(err).__name__, err))

def get_xml_header_value(root:ET.Element, tag:str):
    if root:
        header = root.find('header')
        if header:
            try:
                return header.find(tag).text
            except AttributeError:
                return
        else:
            print('Node "header" doesn\'t exist in XML file')
    return

def iterate_xml_widgets(root:ET.Element):
    if root:
        body = root.find('body')
        if body:
            for child in body:
                if child.tag == 'widget_element':
                    yield child
        else:
            print('Node "body" doesn\'t exist in XML file')
    return

def get_xml_widget_property(widget_element:ET.Element, tag:str):
    if widget_element:
        try:
            return widget_element.find(tag).text
        except AttributeError:
            return
    return


# decorator
def singleton(cls):
    """Make a class a Singleton class (only one instance)"""
    instances = {}
    @functools.wraps(cls)
    def wrapper_singleton(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]
    return wrapper_singleton


def widgetXML(xml_path:str):
    def decorator_widget(cls):
        class ScriptingWidgetUIBase(object):
            pass

        class WidgetClass(cls):
            def __init__(self, widget_BP, ui, *args, **kwargs):
                self.widget_BP = widget_BP
                self.widget_id = None
                self.ui = ui
                super().__init__(*args, **kwargs)

            def show(self):
                editor_sub = unreal.get_editor_subsystem(unreal.EditorUtilitySubsystem)
                if editor_sub.find_utility_widget_from_blueprint(self.widget_BP):
                    return
                if hasattr(super(), 'show'):
                    super().show()
                if not self.widget_id:
                    widget, self.widget_id = editor_sub.spawn_and_register_tab_and_get_id(self.widget_BP)
                elif not editor_sub.does_tab_exist(self.widget_id):
                    editor_sub.spawn_registered_tab_by_id(self.widget_id)
            
            def close(self):
                if hasattr(super(), 'close'):
                    super().close()
                if self.widget_id:
                    editor_sub = unreal.get_editor_subsystem(unreal.EditorUtilitySubsystem)
                    editor_sub.close_tab_by_id(self.widget_id)

        @functools.wraps(cls)
        def wrapper_widget(*args, **kwargs):
            xml_root = get_xml_root(convert_to_absolute_path(xml_path))
            asset_name = get_xml_header_value(xml_root, 'asset_name')
            asset_path = get_xml_header_value(xml_root, 'asset_path')
            widget_asset = unreal.AssetRegistryHelpers.get_asset_registry().get_asset_by_object_path(asset_path)
            if widget_asset.is_asset_loaded():
                unreal.EditorLoadingAndSavingUtils.reload_packages([unreal.load_package(widget_asset.package_name)])
            widget_BP = unreal.load_asset(asset_path)
            ui_object = ScriptingWidgetUIBase()
            for widget_element in iterate_xml_widgets(xml_root):
                widget_name = widget_element.get('name')
                widget_object = unreal.find_object(None, '{}_C:WidgetTree.{}'.format(asset_path, widget_name))
                ui_object.__setattr__(widget_name, widget_object)
            widget_asset_obj = WidgetClass(widget_BP, ui_object, *args, **kwargs)
            widget_asset_obj.__name__ = cls.__name__
            return widget_asset_obj
        return wrapper_widget
    return decorator_widget
