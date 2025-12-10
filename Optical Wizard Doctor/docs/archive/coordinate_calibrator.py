#!/usr/bin/env python3
"""
Coordinate Calibrator Tool
Visual tool to select regions on PDF and extract coordinates for OCR extraction.
"""

from pathlib import Path
import json
import tkinter as tk
from tkinter import filedialog, messagebox, ttk, simpledialog
from PIL import Image, ImageTk
from pdf2image import convert_from_path
import numpy as np

class CoordinateCalibrator:
    """GUI tool for selecting and calibrating extraction regions on PDFs."""
    
    def __init__(self, root):
        self.root = root
        self.root.title("PDF Coordinate Calibrator")
        self.root.geometry("1200x800")
        self.root.minsize(800, 600)  # Minimum window size
        
        # State
        self.pdf_path = None
        self.image = None
        self.photo = None
        self.scale_factor = 1.0
        self.regions = {}  # name -> {coords, color, group, etc.}
        self.groups = {}  # group_name -> [region_names]
        self.current_region = None
        self.start_x = None
        self.start_y = None
        self.rectangles = []  # Canvas rectangle IDs
        self.selected_region = None  # Single selection (for backward compatibility)
        self.selected_regions = set()  # Multi-selection set
        self.dragging = False
        self.resizing = False
        self.resize_handle = None
        self.selection_box = None  # Selection box for drag selection
        self.selection_box_start = None  # Start point for selection box
        self.mask_enabled = False
        self.mask_overlay = None
        self.mouse_mode = "create"  # "create" or "select"
        self.zoom_level = 1.0
        self.canvas_offset_x = 0
        self.canvas_offset_y = 0
        self.image_x = 0  # Image position on canvas
        self.image_y = 0  # Image position on canvas
        self.hovered_region = None  # Track which region is being hovered
        self.resize_timer = None  # Timer for debouncing resize events
        # Undo/Redo system
        self.undo_stack = []  # Stack of previous states
        self.redo_stack = []  # Stack of undone states
        self.max_undo_history = 50  # Maximum undo history
        
        # UI Setup
        self.setup_ui()
        
    def setup_ui(self):
        """Set up the user interface."""
        # Toolbar frame
        toolbar_frame = tk.Frame(self.root, bg="#e0e0e0", relief=tk.RAISED, bd=1)
        toolbar_frame.pack(fill=tk.X, padx=0, pady=0)
        
        # Mouse mode buttons
        mode_frame = tk.Frame(toolbar_frame, bg="#e0e0e0")
        mode_frame.pack(side=tk.LEFT, padx=5, pady=3)
        
        tk.Label(mode_frame, text="Mode:", bg="#e0e0e0", fg="black", font=("Arial", 9, "bold")).pack(side=tk.LEFT, padx=2)
        
        self.mode_var = tk.StringVar(value="create")
        create_btn = tk.Radiobutton(mode_frame, text="Create", variable=self.mode_var, value="create",
                                    command=self.set_mode_create, bg="#e0e0e0", fg="black", indicatoron=0,
                                    selectcolor="#90caf9", width=8, activebackground="#e0e0e0", 
                                    activeforeground="black")
        create_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(create_btn, "Create new regions by clicking and dragging")
        
        select_btn = tk.Radiobutton(mode_frame, text="Select/Move", variable=self.mode_var, value="select",
                                    command=self.set_mode_select, bg="#e0e0e0", fg="black", indicatoron=0,
                                    selectcolor="#90caf9", width=10, activebackground="#e0e0e0",
                                    activeforeground="black")
        select_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(select_btn, "Select and move existing regions")
        
        # Zoom controls
        zoom_frame = tk.Frame(toolbar_frame, bg="#e0e0e0")
        zoom_frame.pack(side=tk.LEFT, padx=10, pady=3)
        
        tk.Label(zoom_frame, text="Zoom:", bg="#e0e0e0", fg="black", font=("Arial", 9, "bold")).pack(side=tk.LEFT, padx=2)
        
        zoom_out_btn = tk.Button(zoom_frame, text="−", command=self.zoom_out, width=3, fg="black")
        zoom_out_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(zoom_out_btn, "Zoom out")
        
        self.zoom_label = tk.Label(zoom_frame, text="100%", bg="#e0e0e0", fg="black", width=6)
        self.zoom_label.pack(side=tk.LEFT, padx=2)
        
        zoom_in_btn = tk.Button(zoom_frame, text="+", command=self.zoom_in, width=3, fg="black")
        zoom_in_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(zoom_in_btn, "Zoom in")
        
        reset_zoom_btn = tk.Button(zoom_frame, text="Reset", command=self.reset_zoom, width=6, fg="black")
        reset_zoom_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(reset_zoom_btn, "Reset zoom to fit")
        
        # Group/Ungroup button
        group_frame = tk.Frame(toolbar_frame, bg="#e0e0e0")
        group_frame.pack(side=tk.LEFT, padx=10, pady=3)
        
        group_btn = tk.Button(group_frame, text="Group Selected", command=self.quick_group, width=12, fg="black")
        group_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(group_btn, "Group selected regions")
        
        ungroup_btn = tk.Button(group_frame, text="Ungroup Selected", command=self.quick_ungroup, width=12, fg="black")
        ungroup_btn.pack(side=tk.LEFT, padx=2)
        self.create_tooltip(ungroup_btn, "Ungroup selected regions")
        
        # Separator
        tk.Frame(toolbar_frame, bg="#b0b0b0", width=1).pack(side=tk.LEFT, fill=tk.Y, padx=5, pady=3)
        
        # Top frame for controls
        control_frame = tk.Frame(self.root)
        control_frame.pack(fill=tk.X, padx=10, pady=5)
        
        # File selection
        load_btn = tk.Button(control_frame, text="Load PDF", command=self.load_pdf, fg="black")
        load_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(load_btn, "Load a PDF file to calibrate")
        
        self.file_label = tk.Label(control_frame, text="No file loaded")
        self.file_label.pack(side=tk.LEFT, padx=10)
        
        # Region name input
        tk.Label(control_frame, text="Region Name:").pack(side=tk.LEFT, padx=5)
        self.region_name_entry = tk.Entry(control_frame, width=20)
        self.region_name_entry.pack(side=tk.LEFT, padx=5)
        self.region_name_entry.insert(0, "Last Name Cell 1")
        
        # Group input
        tk.Label(control_frame, text="Group:").pack(side=tk.LEFT, padx=5)
        self.group_entry = tk.Entry(control_frame, width=15)
        self.group_entry.pack(side=tk.LEFT, padx=5)
        self.group_entry.insert(0, "lastname")
        
        # Color selection
        tk.Label(control_frame, text="Color:").pack(side=tk.LEFT, padx=5)
        self.color_var = tk.StringVar(value="blue")
        color_menu = ttk.Combobox(control_frame, textvariable=self.color_var, width=10, 
                                  values=["blue", "red", "green", "yellow", "purple", "orange", "cyan"])
        color_menu.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(color_menu, "Color for new regions")
        
        # Mask toggle
        self.mask_var = tk.BooleanVar()
        mask_cb = tk.Checkbutton(control_frame, text="Mask Outside", variable=self.mask_var, 
                      command=self.toggle_mask)
        mask_cb.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(mask_cb, "Grey out areas outside selected regions")
        
        # Buttons
        clear_btn = tk.Button(control_frame, text="Clear Selection", command=self.clear_selection, fg="black")
        clear_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(clear_btn, "Clear current selection")
        
        edit_btn = tk.Button(control_frame, text="Edit Selected", command=self.edit_selected_region, fg="black")
        edit_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(edit_btn, "Edit selected region name")
        
        save_btn = tk.Button(control_frame, text="Export Coordinates", command=self.export_coordinates, fg="black")
        save_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(save_btn, "Export coordinates in multiple formats (JSON, CSV, TXT, etc.)")
        
        mask_btn = tk.Button(control_frame, text="Export Mask", command=self.export_mask, fg="black")
        mask_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(mask_btn, "Export mask image for OCR (blacks out everything except labeled cells)")
        
        load_btn2 = tk.Button(control_frame, text="Load Coordinates", command=self.load_coordinates, fg="black")
        load_btn2.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(load_btn2, "Load coordinates from JSON file")
        
        test_btn = tk.Button(control_frame, text="Test Extraction", command=self.test_extraction, fg="black")
        test_btn.pack(side=tk.LEFT, padx=5)
        self.create_tooltip(test_btn, "Test OCR extraction on regions")
        
        # Main frame with canvas and list
        main_frame = tk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        # Left: Canvas for PDF display
        canvas_frame = tk.Frame(main_frame, bg="#d0d0d0")
        canvas_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        status_label = tk.Label(canvas_frame, text="Click and drag to create regions", 
                                font=("Arial", 10), bg="#d0d0d0", fg="black")
        status_label.pack()
        self.status_label = status_label
        
        # Canvas with scrollbars
        canvas_container = tk.Frame(canvas_frame, bg="#d0d0d0")
        canvas_container.pack(fill=tk.BOTH, expand=True)
        
        # Scrollbars
        v_scrollbar = tk.Scrollbar(canvas_container, orient=tk.VERTICAL)
        v_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        h_scrollbar = tk.Scrollbar(canvas_container, orient=tk.HORIZONTAL)
        h_scrollbar.pack(side=tk.BOTTOM, fill=tk.X)
        
        self.canvas = tk.Canvas(canvas_container, bg="#d0d0d0", cursor="crosshair",
                                yscrollcommand=v_scrollbar.set, xscrollcommand=h_scrollbar.set)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        v_scrollbar.config(command=self.canvas.yview)
        h_scrollbar.config(command=self.canvas.xview)
        
        # Bind scroll wheel with modifiers
        # Plain scroll = vertical scrolling
        self.canvas.bind("<MouseWheel>", self.on_mousewheel)
        # Shift+scroll = horizontal scrolling (Magic Mouse side-to-side)
        self.canvas.bind("<Shift-MouseWheel>", self.on_mousewheel_shift)
        # Command+scroll = zoom (both horizontal and vertical)
        self.canvas.bind("<Control-MouseWheel>", self.on_mousewheel_cmd)
        # Linux support
        self.canvas.bind("<Button-4>", self.on_mousewheel)
        self.canvas.bind("<Button-5>", self.on_mousewheel)
        self.canvas.bind("<Shift-Button-4>", self.on_mousewheel_shift)
        self.canvas.bind("<Shift-Button-5>", self.on_mousewheel_shift)
        self.canvas.bind("<Control-Button-4>", self.on_mousewheel_cmd)
        self.canvas.bind("<Control-Button-5>", self.on_mousewheel_cmd)
        
        # Bind keyboard shortcuts
        self.setup_hotkeys()
        
        # Bind mouse events
        self.canvas.bind("<Button-1>", self.on_click)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_release)
        self.canvas.bind("<Double-Button-1>", self.on_double_click)
        self.canvas.bind("<Button-3>", self.on_right_click)  # Right-click for context menu
        # Command+click for multi-select
        self.canvas.bind("<Control-Button-1>", self.on_cmd_click)
        # Shift+click for box selection start
        self.canvas.bind("<Shift-Button-1>", self.on_shift_click)
        self.canvas.bind("<Shift-B1-Motion>", self.on_shift_drag)
        self.canvas.bind("<Shift-ButtonRelease-1>", self.on_shift_release)
        self.canvas.bind("<Motion>", self.on_mouse_move)
        self.canvas.bind("<Leave>", self.on_canvas_leave)  # Clear hover when leaving canvas
        self.canvas.bind("<Configure>", self.on_canvas_configure)
        
        # Focus on canvas to receive keyboard events
        self.canvas.focus_set()
        
        # Right: Region list and info
        right_frame = tk.Frame(main_frame, width=300)
        right_frame.pack(side=tk.RIGHT, fill=tk.Y, padx=10)
        right_frame.pack_propagate(False)
        
        # Notebook for tabs
        notebook = ttk.Notebook(right_frame)
        notebook.pack(fill=tk.BOTH, expand=True)
        
        # Regions tab
        regions_tab = tk.Frame(notebook)
        notebook.add(regions_tab, text="Regions")
        
        tk.Label(regions_tab, text="Defined Regions", font=("Arial", 12, "bold")).pack(pady=5)
        
        # Listbox for regions
        list_frame = tk.Frame(regions_tab)
        list_frame.pack(fill=tk.BOTH, expand=True)
        
        scrollbar = tk.Scrollbar(list_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.region_listbox = tk.Listbox(list_frame, yscrollcommand=scrollbar.set)
        self.region_listbox.pack(fill=tk.BOTH, expand=True)
        self.region_listbox.bind("<<ListboxSelect>>", self.on_region_select)
        scrollbar.config(command=self.region_listbox.yview)
        
        # Region buttons
        btn_frame = tk.Frame(regions_tab)
        btn_frame.pack(fill=tk.X, pady=5)
        tk.Button(btn_frame, text="Edit Name", command=self.edit_region_name, fg="black").pack(side=tk.LEFT, padx=2)
        tk.Button(btn_frame, text="Change Color", command=self.change_region_color, fg="black").pack(side=tk.LEFT, padx=2)
        tk.Button(btn_frame, text="Delete", command=self.delete_region, bg="#ff6b6b", fg="black").pack(side=tk.LEFT, padx=2)
        
        # Groups tab
        groups_tab = tk.Frame(notebook)
        notebook.add(groups_tab, text="Groups")
        
        tk.Label(groups_tab, text="Region Groups", font=("Arial", 12, "bold")).pack(pady=5)
        
        # Group list
        group_list_frame = tk.Frame(groups_tab)
        group_list_frame.pack(fill=tk.BOTH, expand=True)
        
        group_scrollbar = tk.Scrollbar(group_list_frame)
        group_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.group_listbox = tk.Listbox(group_list_frame, yscrollcommand=group_scrollbar.set)
        self.group_listbox.pack(fill=tk.BOTH, expand=True)
        self.group_listbox.bind("<<ListboxSelect>>", self.on_group_select)
        group_scrollbar.config(command=self.group_listbox.yview)
        
        # Group buttons
        group_btn_frame = tk.Frame(groups_tab)
        group_btn_frame.pack(fill=tk.X, pady=5)
        tk.Button(group_btn_frame, text="Create Group", command=self.create_group, fg="black").pack(side=tk.LEFT, padx=2)
        tk.Button(group_btn_frame, text="Add to Group", command=self.add_to_group, fg="black").pack(side=tk.LEFT, padx=2)
        tk.Button(group_btn_frame, text="Remove from Group", command=self.remove_from_group, fg="black").pack(side=tk.LEFT, padx=2)
        tk.Button(group_btn_frame, text="Delete Group", command=self.delete_group, bg="#ff6b6b", fg="black").pack(side=tk.LEFT, padx=2)
        
        # Info display
        self.info_text = tk.Text(right_frame, height=10, wrap=tk.WORD)
        self.info_text.pack(fill=tk.BOTH, expand=True, pady=5)
        
        # Help button (?) to show instructions
        help_btn = tk.Button(right_frame, text="?", command=self.show_help, 
                            width=3, height=1, font=("Arial", 12, "bold"), 
                            bg="#e0e0e0", fg="black", relief=tk.RAISED)
        help_btn.pack(pady=5)
        self.create_tooltip(help_btn, "Show help and instructions")
    
    def show_help(self):
        """Show help dialog with instructions and hotkeys."""
        help_window = tk.Toplevel(self.root)
        help_window.title("Help & Instructions")
        help_window.geometry("600x700")
        help_window.resizable(True, True)
        
        # Create scrollable text widget
        frame = tk.Frame(help_window)
        frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        scrollbar = tk.Scrollbar(frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        text_widget = tk.Text(frame, wrap=tk.WORD, yscrollcommand=scrollbar.set,
                             font=("Arial", 10), padx=10, pady=10)
        text_widget.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=text_widget.yview)
        
        instructions = """COORDINATE CALIBRATOR - HELP & INSTRUCTIONS
═══════════════════════════════════════════════════════════════

BASIC WORKFLOW:
1. Load a PDF file (Cmd+O)
2. Enter region name (e.g., "Last Name Cell 1")
3. Click and drag on image to select region
4. Region will be saved with name
5. Repeat for all character cells
6. Export coordinates (Cmd+S)
7. Test extraction (T)

SELECTION MODE:
• Click region to select
• Cmd+Click to toggle multi-select
• Shift+Drag for box selection
• Click empty space + drag for selection box
• Drag selected regions to move

KEYBOARD SHORTCUTS:

Mode Switching:
  1/2          - Switch Create/Select mode

Zoom:
  +/-          - Zoom in/out
  0            - Reset zoom

File Operations:
  Cmd+O        - Load PDF
  Cmd+S        - Export coordinates (JSON/CSV/TXT/XML/YAML)
  Cmd+Shift+S  - Load coordinates
  Cmd+M        - Export mask image

Region Operations:
  Delete/Backspace - Delete selected regions
  E            - Edit selected region name
  Cmd+Z        - Undo
  Cmd+Shift+Z  - Redo (or Cmd+Y)

Group Operations:
  Cmd+G        - Group selected regions
  Cmd+Shift+G  - Ungroup selected regions

Selection Operations:
  Cmd+A        - Select all regions
  Cmd+D        - Deselect all regions
  Cmd+I        - Invert selection

Other:
  M            - Toggle mask overlay
  T            - Test OCR extraction

SCROLL BEHAVIOR:
  Scroll       - Vertical scroll
  Shift+Scroll - Horizontal scroll (side-to-side)
  Cmd+Scroll   - Zoom in/out (horizontal or vertical)

EXPORT FORMATS:
The tool exports normalized coordinates (percentages 0.0-1.0) that work
with any document of the same aspect ratio, regardless of pixel size.

Supported formats:
• JSON - Structured data format
• CSV  - Spreadsheet-friendly format
• TXT  - Human-readable plain text
• XML  - Structured XML format
• YAML - YAML format (requires PyYAML)

MASK EXPORT:
The mask image blacks out everything except labeled cells, which can
be used as an overlay during OCR to improve accuracy by hiding
unnecessary information.
"""
        
        text_widget.insert(1.0, instructions)
        text_widget.config(state=tk.DISABLED)
        
        # Close button
        close_btn = tk.Button(help_window, text="Close", command=help_window.destroy, fg="black")
        close_btn.pack(pady=10)
    
    def save_state(self):
        """Save current state for undo/redo."""
        import copy
        state = {
            'regions': copy.deepcopy(self.regions),
            'groups': copy.deepcopy(self.groups)
        }
        self.undo_stack.append(state)
        # Limit undo history
        if len(self.undo_stack) > self.max_undo_history:
            self.undo_stack.pop(0)
        # Clear redo stack when new action is performed
        self.redo_stack = []
        self.update_undo_redo_buttons()
    
    def restore_state(self, state):
        """Restore state from undo/redo."""
        import copy
        self.regions = copy.deepcopy(state['regions'])
        self.groups = copy.deepcopy(state['groups'])
        # Recalculate canvas coordinates
        self.update_region_canvas_coords()
        self.update_region_list()
        self.update_group_list()
        self.redraw_regions()
        self.selected_region = None
        self.selected_regions = set()
    
    def undo_action(self):
        """Undo last action."""
        if not self.undo_stack:
            return
        
        # Save current state to redo stack
        import copy
        current_state = {
            'regions': copy.deepcopy(self.regions),
            'groups': copy.deepcopy(self.groups)
        }
        self.redo_stack.append(current_state)
        
        # Restore previous state
        previous_state = self.undo_stack.pop()
        self.restore_state(previous_state)
        self.update_undo_redo_buttons()
    
    def redo_action(self):
        """Redo last undone action."""
        if not self.redo_stack:
            return
        
        # Save current state to undo stack
        import copy
        current_state = {
            'regions': copy.deepcopy(self.regions),
            'groups': copy.deepcopy(self.groups)
        }
        self.undo_stack.append(current_state)
        
        # Restore next state
        next_state = self.redo_stack.pop()
        self.restore_state(next_state)
        self.update_undo_redo_buttons()
    
    def update_undo_redo_buttons(self):
        """Update undo/redo button states."""
        if hasattr(self, 'undo_btn'):
            self.undo_btn.config(state=tk.NORMAL if self.undo_stack else tk.DISABLED)
        if hasattr(self, 'redo_btn'):
            self.redo_btn.config(state=tk.NORMAL if self.redo_stack else tk.DISABLED)
    
    def create_tooltip(self, widget, text):
        """Create a tooltip for a widget."""
        def on_enter(event):
            tooltip = tk.Toplevel()
            tooltip.wm_overrideredirect(True)
            tooltip.wm_geometry(f"+{event.x_root+10}+{event.y_root+10}")
            label = tk.Label(tooltip, text=text, background="#ffffe0", 
                           foreground="black", relief=tk.SOLID, borderwidth=1, 
                           font=("Arial", 9), padx=5, pady=2)
            label.pack()
            widget.tooltip = tooltip
        
        def on_leave(event):
            if hasattr(widget, 'tooltip'):
                widget.tooltip.destroy()
                del widget.tooltip
        
        widget.bind("<Enter>", on_enter)
        widget.bind("<Leave>", on_leave)
    
    def set_mode_create(self):
        """Set mouse mode to create."""
        self.mouse_mode = "create"
        self.canvas.config(cursor="crosshair")
        self.status_label.config(text="Mode: Create - Click and drag to create new regions")
        self.selected_region = None
        self.selected_regions = set()
        self.redraw_regions()
    
    def set_mode_select(self):
        """Set mouse mode to select/move."""
        self.mouse_mode = "select"
        self.canvas.config(cursor="arrow")
        self.status_label.config(text="Mode: Select/Move - Click to select, Cmd+Click to multi-select, Shift+Drag for box select")
    
    def zoom_in(self):
        """Zoom in on the canvas."""
        self.zoom_level = min(self.zoom_level * 1.2, 5.0)
        self.update_zoom_display()
        self.display_image()
    
    def zoom_out(self):
        """Zoom out on the canvas."""
        self.zoom_level = max(self.zoom_level / 1.2, 0.2)
        self.update_zoom_display()
        self.display_image()
    
    def reset_zoom(self):
        """Reset zoom to fit."""
        self.zoom_level = 1.0
        self.canvas_offset_x = 0
        self.canvas_offset_y = 0
        self.update_zoom_display()
        self.display_image()
    
    def update_zoom_display(self):
        """Update zoom percentage label."""
        self.zoom_label.config(text=f"{int(self.zoom_level * 100)}%", fg="black")
    
    def setup_hotkeys(self):
        """Set up keyboard shortcuts."""
        # Mode switching
        self.root.bind("<KeyPress-1>", lambda e: self.set_mode_create())
        self.root.bind("<KeyPress-2>", lambda e: self.set_mode_select())
        
        # Zoom
        self.root.bind("<KeyPress-equal>", lambda e: self.zoom_in())  # = key
        self.root.bind("<KeyPress-plus>", lambda e: self.zoom_in())  # + key
        self.root.bind("<KeyPress-minus>", lambda e: self.zoom_out())  # - key
        self.root.bind("<KeyPress-0>", lambda e: self.reset_zoom())  # 0 key
        
        # File operations
        self.root.bind("<Control-o>", lambda e: self.load_pdf())  # Cmd+O
        self.root.bind("<Control-s>", lambda e: self.export_coordinates())  # Cmd+S
        self.root.bind("<Control-Shift-S>", lambda e: self.load_coordinates())  # Cmd+Shift+S
        self.root.bind("<Control-m>", lambda e: self.export_mask())  # Cmd+M for mask
        
        # Region operations
        self.root.bind("<Delete>", lambda e: self.delete_selected_region())
        self.root.bind("<BackSpace>", lambda e: self.delete_selected_region())
        self.root.bind("<KeyPress-e>", lambda e: self.edit_selected_region())
        
        # Group operations
        self.root.bind("<Control-g>", lambda e: self.quick_group())  # Cmd+G
        self.root.bind("<Control-Shift-G>", lambda e: self.quick_ungroup())  # Cmd+Shift+G
        
        # Mask toggle
        self.root.bind("<KeyPress-m>", lambda e: self.toggle_mask_keyboard())
        
        # Test extraction
        self.root.bind("<KeyPress-t>", lambda e: self.test_extraction())
        
        # Selection operations
        self.root.bind("<Control-a>", lambda e: self.select_all())  # Cmd+A
        self.root.bind("<Control-d>", lambda e: self.deselect_all())  # Cmd+D
        self.root.bind("<Control-i>", lambda e: self.invert_selection())  # Cmd+I
        
        # Undo/Redo
        self.root.bind("<Control-z>", lambda e: self.undo_action())  # Cmd+Z
        self.root.bind("<Control-Shift-Z>", lambda e: self.redo_action())  # Cmd+Shift+Z
        self.root.bind("<Control-y>", lambda e: self.redo_action())  # Cmd+Y (alternative redo)
    
    def on_mousewheel(self, event):
        """Handle mouse wheel for vertical scrolling (default behavior)."""
        # Plain scroll = vertical scrolling
        if event.delta > 0 or event.num == 4:
            self.canvas.yview_scroll(-1, "units")
        elif event.delta < 0 or event.num == 5:
            self.canvas.yview_scroll(1, "units")
    
    def on_mousewheel_shift(self, event):
        """Handle Shift+scroll for horizontal scrolling (Magic Mouse side-to-side)."""
        # Shift+scroll = horizontal scrolling (side-to-side)
        if event.delta > 0 or event.num == 4:
            self.canvas.xview_scroll(-1, "units")  # Scroll left
        elif event.delta < 0 or event.num == 5:
            self.canvas.xview_scroll(1, "units")  # Scroll right
    
    def on_mousewheel_cmd(self, event):
        """Handle Command+scroll for zoom (both horizontal and vertical)."""
        # Command+scroll = zoom in/out (works for both horizontal and vertical scroll)
        if event.delta > 0 or event.num == 4:
            self.zoom_in()
        elif event.delta < 0 or event.num == 5:
            self.zoom_out()
    
    def on_cmd_click(self, event):
        """Handle Command+click for multi-select toggle."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        if self.mouse_mode != "select":
            return
        
        clicked_region = self.get_region_at(x, y)
        if clicked_region:
            # Toggle selection
            if clicked_region in self.selected_regions:
                # Deselect
                self.selected_regions.discard(clicked_region)
                if self.selected_region == clicked_region:
                    self.selected_region = list(self.selected_regions)[0] if self.selected_regions else None
            else:
                # Select
                self.selected_regions.add(clicked_region)
                self.selected_region = clicked_region
            self.redraw_regions()
    
    def on_shift_click(self, event):
        """Handle Shift+click to start box selection."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        if self.mouse_mode != "select":
            return
        
        # Start box selection
        self.selection_box_start = (x, y)
        self.start_x = x
        self.start_y = y
    
    def on_shift_drag(self, event):
        """Handle Shift+drag for box selection."""
        if not self.selection_box_start:
            return
        
        cur_x = self.canvas.canvasx(event.x)
        cur_y = self.canvas.canvasy(event.y)
        
        # Delete old selection box
        if self.selection_box:
            self.canvas.delete(self.selection_box)
        
        # Draw new selection box
        self.selection_box = self.canvas.create_rectangle(
            self.selection_box_start[0], self.selection_box_start[1], cur_x, cur_y,
            outline="#0066ff", width=2, dash=(5, 5), tags="selection_box"
        )
    
    def on_shift_release(self, event):
        """Handle Shift+release to finalize box selection."""
        if not self.selection_box_start:
            return
        
        end_x = self.canvas.canvasx(event.x)
        end_y = self.canvas.canvasy(event.y)
        
        # Find all regions within selection box
        x1, y1 = self.selection_box_start
        x2, y2 = end_x, end_y
        box_x_min, box_x_max = min(x1, x2), max(x1, x2)
        box_y_min, box_y_max = min(y1, y2), max(y1, y2)
        
        # Select all regions that intersect with the box
        selected = set()
        for name, data in self.regions.items():
            reg_x1, reg_y1, reg_x2, reg_y2 = data['canvas_coords']
            reg_x_min, reg_x_max = min(reg_x1, reg_x2), max(reg_x1, reg_x2)
            reg_y_min, reg_y_max = min(reg_y1, reg_y2), max(reg_y1, reg_y2)
            
            # Check if region intersects with selection box
            if not (reg_x_max < box_x_min or reg_x_min > box_x_max or 
                    reg_y_max < box_y_min or reg_y_min > box_y_max):
                selected.add(name)
        
        # Add to existing selection (Shift adds to selection)
        self.selected_regions.update(selected)
        if selected:
            self.selected_region = list(selected)[0]
        
        # Clean up selection box
        if self.selection_box:
            self.canvas.delete(self.selection_box)
            self.selection_box = None
        self.selection_box_start = None
        self.redraw_regions()
    
    def delete_selected_region(self):
        """Delete the currently selected regions via keyboard."""
        if self.selected_regions:
            # Delete all selected regions
            for region_name in list(self.selected_regions):
                self.delete_region_by_name(region_name)
            self.selected_regions = set()
            self.selected_region = None
        elif self.selected_region:
            self.delete_region_by_name(self.selected_region)
        else:
            # Try to delete from listbox selection
            selection = self.region_listbox.curselection()
            if selection:
                region_name = self.region_listbox.get(selection[0])
                self.delete_region_by_name(region_name)
    
    def select_all(self):
        """Select all regions."""
        self.selected_regions = set(self.regions.keys())
        self.selected_region = list(self.selected_regions)[0] if self.selected_regions else None
        self.redraw_regions()
    
    def deselect_all(self):
        """Deselect all regions."""
        self.selected_regions = set()
        self.selected_region = None
        self.redraw_regions()
    
    def invert_selection(self):
        """Invert the current selection."""
        all_regions = set(self.regions.keys())
        self.selected_regions = all_regions - self.selected_regions
        self.selected_region = list(self.selected_regions)[0] if self.selected_regions else None
        self.redraw_regions()
    
    def toggle_mask_keyboard(self):
        """Toggle mask via keyboard."""
        self.mask_var.set(not self.mask_var.get())
        self.toggle_mask()
    
    def on_canvas_configure(self, event):
        """Handle canvas resize with debouncing to prevent freezing."""
        # Cancel any pending resize operation
        if self.resize_timer:
            self.root.after_cancel(self.resize_timer)
        
        # Schedule a delayed redraw (debounce with longer delay)
        # This prevents the expensive display_image() from being called
        # on every single resize event during window dragging
        # Increased delay to 300ms for better stability
        self.resize_timer = self.root.after(300, self._delayed_resize)
    
    def _delayed_resize(self):
        """Perform the actual resize after debounce delay."""
        self.resize_timer = None
        if self.image:
            self.display_image()
    
    def quick_group(self):
        """Quick group selected regions."""
        selection = self.region_listbox.curselection()
        if len(selection) < 2:
            messagebox.showwarning("Warning", "Select at least 2 regions to group")
            return
        
        group_name = simpledialog.askstring("Group Regions", "Enter group name:")
        if not group_name:
            return
        
        if group_name not in self.groups:
            self.groups[group_name] = []
        
        for idx in selection:
            region_name = self.region_listbox.get(idx)
            if region_name not in self.groups[group_name]:
                self.groups[group_name].append(region_name)
            self.regions[region_name]['group'] = group_name
        
        self.update_group_list()
        self.redraw_regions()
        messagebox.showinfo("Success", f"Grouped {len(selection)} regions as '{group_name}'")
    
    def quick_ungroup(self):
        """Quick ungroup selected regions."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Select regions to ungroup")
            return
        
        ungrouped = 0
        for idx in selection:
            region_name = self.region_listbox.get(idx)
            if region_name in self.regions:
                group = self.regions[region_name].get('group')
                if group and group in self.groups:
                    if region_name in self.groups[group]:
                        self.groups[group].remove(region_name)
                    if not self.groups[group]:
                        del self.groups[group]
                self.regions[region_name]['group'] = None
                ungrouped += 1
        
        self.update_group_list()
        self.redraw_regions()
        messagebox.showinfo("Success", f"Ungrouped {ungrouped} regions")
        
    def load_pdf(self):
        """Load a PDF file and display it."""
        file_path = filedialog.askopenfilename(
            title="Select PDF",
            filetypes=[("PDF files", "*.pdf"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        self.pdf_path = file_path
        self.file_label.config(text=Path(file_path).name)
        
        try:
            # Convert PDF to image
            images = convert_from_path(file_path, first_page=1, last_page=1, dpi=150)
            if not images:
                messagebox.showerror("Error", "Could not convert PDF to image")
                return
            
            self.image = images[0]
            self.display_image()
            self.regions = {}
            self.update_region_list()
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load PDF: {e}")
    
    def display_image(self):
        """Display the image on canvas with shadow and proper background."""
        if not self.image:
            return
        
        # Get canvas size (without update() to avoid blocking)
        try:
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()
        except:
            return
        
        if canvas_width <= 1 or canvas_height <= 1:
            # Canvas not ready, schedule retry
            self.root.after(100, self.display_image)
            return
        
        # Calculate scale with zoom
        img_width, img_height = self.image.size
        base_scale_w = canvas_width / img_width
        base_scale_h = canvas_height / img_height
        base_scale = min(base_scale_w, base_scale_h, 1.0)
        
        # Apply zoom level
        self.scale_factor = base_scale * self.zoom_level
        
        # Resize image
        display_width = int(img_width * self.scale_factor)
        display_height = int(img_height * self.scale_factor)
        display_image = self.image.resize((display_width, display_height), Image.Resampling.LANCZOS)
        
        # Convert to PhotoImage
        self.photo = ImageTk.PhotoImage(display_image)
        
        # Clear canvas
        self.canvas.delete("all")
        
        # Calculate centered position
        self.image_x = (canvas_width - display_width) // 2 + self.canvas_offset_x
        self.image_y = (canvas_height - display_height) // 2 + self.canvas_offset_y
        
        # Draw shadow (offset by 5 pixels)
        shadow_offset = 5
        shadow_x = self.image_x + shadow_offset
        shadow_y = self.image_y + shadow_offset
        
        # Create shadow rectangle
        self.canvas.create_rectangle(
            shadow_x, shadow_y,
            shadow_x + display_width, shadow_y + display_height,
            fill="#808080", outline="", tags="shadow"
        )
        
        # Draw image on top of shadow
        self.canvas.create_image(self.image_x, self.image_y, image=self.photo, anchor=tk.NW, tags="image")
        
        # Update scroll region
        self.canvas.config(scrollregion=self.canvas.bbox("all"))
        
        # Redraw all regions
        self.redraw_regions()
    
    def on_click(self, event):
        """Handle mouse click - start selection or drag/resize."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        # Clear hover state on click
        if self.hovered_region:
            self.hovered_region = None
        
        # In select mode, prioritize selection/movement
        if self.mouse_mode == "select":
            # Check if clicking on a resize handle
            items = self.canvas.find_overlapping(x-5, y-5, x+5, y+5)
            for item in items:
                tags = self.canvas.gettags(item)
                if "resize_handle" in tags:
                    self.resizing = True
                    self.selected_region = self.get_region_for_handle(x, y)
                    self.selected_regions = {self.selected_region} if self.selected_region else set()
                    self.resize_handle = self.get_handle_position(x, y)
                    self.redraw_regions()
                    return
            
            # Check if clicking on an existing region
            clicked_region = self.get_region_at(x, y)
            if clicked_region:
                # Single click selects one region
                self.selected_region = clicked_region
                self.selected_regions = {clicked_region}
                self.dragging = True
                self.start_x = x
                self.start_y = y
                self.redraw_regions()
                return
            else:
                # Clicked on empty space - start selection box
                self.selected_region = None
                self.selected_regions = set()
                self.selection_box_start = (x, y)
                self.start_x = x
                self.start_y = y
                self.redraw_regions()
                return
        
        # In create mode, start new selection
        self.selected_region = None
        self.selected_regions = set()
        self.start_x = x
        self.start_y = y
        
        # Delete temporary rectangle if exists
        if self.current_region:
            self.canvas.delete(self.current_region)
    
    def get_region_at(self, x, y):
        """Get region name at given coordinates."""
        items = self.canvas.find_overlapping(x-2, y-2, x+2, y+2)
        for item in items:
            tags = self.canvas.gettags(item)
            for tag in tags:
                if tag in self.regions and tag != "region":
                    return tag
        return None
    
    def get_region_for_handle(self, x, y):
        """Get region that owns the resize handle at given coordinates."""
        # Find the selected region
        return self.selected_region
    
    def get_handle_position(self, x, y):
        """Determine which handle is being dragged."""
        if not self.selected_region or self.selected_region not in self.regions:
            return None
        
        data = self.regions[self.selected_region]
        x1, y1, x2, y2 = data['canvas_coords']
        
        handle_size = 8
        tolerance = handle_size + 5
        
        # Check corners
        if abs(x - x1) < tolerance and abs(y - y1) < tolerance:
            return 'nw'  # Top-left
        elif abs(x - x2) < tolerance and abs(y - y1) < tolerance:
            return 'ne'  # Top-right
        elif abs(x - x1) < tolerance and abs(y - y2) < tolerance:
            return 'sw'  # Bottom-left
        elif abs(x - x2) < tolerance and abs(y - y2) < tolerance:
            return 'se'  # Bottom-right
        # Check edges
        elif abs(x - (x1 + x2) / 2) < tolerance and abs(y - y1) < tolerance:
            return 'n'  # Top
        elif abs(x - (x1 + x2) / 2) < tolerance and abs(y - y2) < tolerance:
            return 's'  # Bottom
        elif abs(x - x1) < tolerance and abs(y - (y1 + y2) / 2) < tolerance:
            return 'w'  # Left
        elif abs(x - x2) < tolerance and abs(y - (y1 + y2) / 2) < tolerance:
            return 'e'  # Right
        
        return None
    
    def on_drag(self, event):
        """Handle mouse drag - update selection rectangle or move/resize region."""
        cur_x = self.canvas.canvasx(event.x)
        cur_y = self.canvas.canvasy(event.y)
        
        # In select mode, handle movement/resizing
        if self.mouse_mode == "select":
            # Handle resizing
            if self.resizing and self.selected_region and self.resize_handle:
                self.resize_region(self.selected_region, self.resize_handle, cur_x, cur_y)
                return
            
            # Handle dragging existing region
            if self.dragging and self.selected_region:
                dx = cur_x - self.start_x
                dy = cur_y - self.start_y
                # Move all selected regions
                for region_name in self.selected_regions:
                    self.move_region(region_name, dx, dy)
                self.start_x = cur_x
                self.start_y = cur_y
                return
            
            # Handle selection box (clicked on empty space)
            if self.selection_box_start:
                # Delete old selection box
                if self.selection_box:
                    self.canvas.delete(self.selection_box)
                
                # Draw new selection box
                self.selection_box = self.canvas.create_rectangle(
                    self.selection_box_start[0], self.selection_box_start[1], cur_x, cur_y,
                    outline="#0066ff", width=2, dash=(5, 5), tags="selection_box"
                )
                return
        
        # In create mode, handle new selection
        if self.mouse_mode == "create":
            if self.start_x is None or self.start_y is None:
                return
            
            # Delete old rectangle
            if self.current_region:
                self.canvas.delete(self.current_region)
            
            # Draw new rectangle
            self.current_region = self.canvas.create_rectangle(
                self.start_x, self.start_y, cur_x, cur_y,
                outline="red", width=2, tags="temp"
            )
    
    def move_region(self, region_name, dx, dy):
        """Move a region by dx, dy pixels."""
        if region_name not in self.regions:
            return
        
        data = self.regions[region_name]
        x1, y1, x2, y2 = data['canvas_coords']
        
        # Update canvas coordinates
        data['canvas_coords'] = [x1 + dx, y1 + dy, x2 + dx, y2 + dy]
        
        # Update image coordinates (canvas coordinates already account for offset)
        img_x1, img_y1, img_x2, img_y2 = data['image_coords']
        img_dx = dx / self.scale_factor
        img_dy = dy / self.scale_factor
        data['image_coords'] = [img_x1 + img_dx, img_y1 + img_dy, img_x2 + img_dx, img_y2 + img_dy]
        
        # Update normalized coordinates
        if self.image:
            img_w, img_h = self.image.size
            norm_dx = img_dx / img_w
            norm_dy = img_dy / img_h
            norm_coords = data['normalized_coords']
            data['normalized_coords'] = [
                norm_coords[0] + norm_dx,
                norm_coords[1] + norm_dy,
                norm_coords[2] + norm_dx,
                norm_coords[3] + norm_dy
            ]
        
        self.redraw_regions()
    
    def resize_region(self, region_name, handle, new_x, new_y):
        """Resize a region using the specified handle."""
        if region_name not in self.regions:
            return
        
        data = self.regions[region_name]
        x1, y1, x2, y2 = data['canvas_coords']
        
        # Update coordinates based on handle
        if handle == 'nw':  # Top-left
            x1, y1 = new_x, new_y
        elif handle == 'ne':  # Top-right
            x2, y1 = new_x, new_y
        elif handle == 'sw':  # Bottom-left
            x1, y2 = new_x, new_y
        elif handle == 'se':  # Bottom-right
            x2, y2 = new_x, new_y
        elif handle == 'n':  # Top
            y1 = new_y
        elif handle == 's':  # Bottom
            y2 = new_y
        elif handle == 'w':  # Left
            x1 = new_x
        elif handle == 'e':  # Right
            x2 = new_x
        
        # Update canvas coordinates
        data['canvas_coords'] = [x1, y1, x2, y2]
        
        # Update image coordinates (accounting for image offset)
        img_x1 = int((x1 - self.image_x) / self.scale_factor)
        img_y1 = int((y1 - self.image_y) / self.scale_factor)
        img_x2 = int((x2 - self.image_x) / self.scale_factor)
        img_y2 = int((y2 - self.image_y) / self.scale_factor)
        data['image_coords'] = [img_x1, img_y1, img_x2, img_y2]
        
        # Update normalized coordinates
        if self.image:
            img_w, img_h = self.image.size
            data['normalized_coords'] = [
                img_x1 / img_w, img_y1 / img_h,
                img_x2 / img_w, img_y2 / img_h
            ]
        
        self.redraw_regions()
    
    def on_release(self, event):
        """Handle mouse release - finalize selection."""
        # Handle selection box in select mode
        if self.mouse_mode == "select" and self.selection_box_start:
            end_x = self.canvas.canvasx(event.x)
            end_y = self.canvas.canvasy(event.y)
            
            # Find all regions within selection box
            x1, y1 = self.selection_box_start
            x2, y2 = end_x, end_y
            box_x_min, box_x_max = min(x1, x2), max(x1, x2)
            box_y_min, box_y_max = min(y1, y2), max(y1, y2)
            
            # Select all regions that intersect with the box
            selected = set()
            for name, data in self.regions.items():
                reg_x1, reg_y1, reg_x2, reg_y2 = data['canvas_coords']
                reg_x_min, reg_x_max = min(reg_x1, reg_x2), max(reg_x1, reg_x2)
                reg_y_min, reg_y_max = min(reg_y1, reg_y2), max(reg_y1, reg_y2)
                
                # Check if region intersects with selection box
                if not (reg_x_max < box_x_min or reg_x_min > box_x_max or 
                        reg_y_max < box_y_min or reg_y_min > box_y_max):
                    selected.add(name)
            
            self.selected_regions = selected
            self.selected_region = list(selected)[0] if selected else None
            
            # Clean up selection box
            if self.selection_box:
                self.canvas.delete(self.selection_box)
                self.selection_box = None
            self.selection_box_start = None
            self.redraw_regions()
            return
        
        # Reset drag/resize state (save state after move/resize)
        if self.dragging or self.resizing:
            # Save state after move/resize is complete
            self.save_state()
            self.dragging = False
            self.resizing = False
            self.resize_handle = None
            self.start_x = None
            self.start_y = None
            return
        
        # Only create new region in create mode
        if self.mouse_mode != "create":
            return
        
        if self.start_x is None or self.start_y is None:
            return
        
        end_x = self.canvas.canvasx(event.x)
        end_y = self.canvas.canvasy(event.y)
        
        # Get region name
        region_name = self.region_name_entry.get().strip()
        if not region_name:
            messagebox.showwarning("Warning", "Please enter a region name")
            return
        
        # Save state before creating new region
        self.save_state()
        
        # Convert canvas coordinates to image coordinates (accounting for image offset)
        img_x1 = int((self.start_x - self.image_x) / self.scale_factor)
        img_y1 = int((self.start_y - self.image_y) / self.scale_factor)
        img_x2 = int((end_x - self.image_x) / self.scale_factor)
        img_y2 = int((end_y - self.image_y) / self.scale_factor)
        
        # Normalize coordinates (0.0-1.0)
        if self.image:
            img_width, img_height = self.image.size
            norm_x1 = img_x1 / img_width
            norm_y1 = img_y1 / img_height
            norm_x2 = img_x2 / img_width
            norm_y2 = img_y2 / img_height
            
            # Get color and group
            color = self.color_var.get()
            group = self.group_entry.get().strip()
            
            # Store region
            self.regions[region_name] = {
                'image_coords': [img_x1, img_y1, img_x2, img_y2],
                'normalized_coords': [norm_x1, norm_y1, norm_x2, norm_y2],
                'canvas_coords': [self.start_x, self.start_y, end_x, end_y],
                'color': color,
                'group': group if group else None
            }
            
            # Add to group if specified
            if group:
                if group not in self.groups:
                    self.groups[group] = []
                if region_name not in self.groups[group]:
                    self.groups[group].append(region_name)
            
            # Clear temporary rectangle
            if self.current_region:
                self.canvas.delete(self.current_region)
                self.current_region = None
            
            # Redraw all regions
            self.redraw_regions()
            self.update_region_list()
            
            # Auto-increment region name
            self.auto_increment_name(region_name)
        
        self.start_x = None
        self.start_y = None
    
    def auto_increment_name(self, current_name):
        """Auto-increment region name (e.g., "Cell 1" -> "Cell 2")."""
        # Try to extract number and increment
        import re
        match = re.search(r'(\d+)$', current_name)
        if match:
            num = int(match.group(1))
            base = current_name[:match.start()]
            self.region_name_entry.delete(0, tk.END)
            self.region_name_entry.insert(0, f"{base}{num + 1}")
        elif "Cell" in current_name or "cell" in current_name:
            # Add number if doesn't exist
            self.region_name_entry.delete(0, tk.END)
            self.region_name_entry.insert(0, f"{current_name} 2")
    
    def update_region_canvas_coords(self):
        """Recalculate canvas coordinates for all regions based on current zoom/scale."""
        if not self.image:
            return
        
        img_w, img_h = self.image.size
        
        for data in self.regions.values():
            # Recalculate canvas coordinates from normalized coordinates
            norm_coords = data.get('normalized_coords', [])
            if norm_coords and len(norm_coords) >= 4:
                data['canvas_coords'] = [
                    norm_coords[0] * img_w * self.scale_factor + self.image_x,
                    norm_coords[1] * img_h * self.scale_factor + self.image_y,
                    norm_coords[2] * img_w * self.scale_factor + self.image_x,
                    norm_coords[3] * img_h * self.scale_factor + self.image_y
                ]
    
    def redraw_regions(self):
        """Redraw all defined regions on canvas."""
        # Update canvas coordinates based on current scale
        self.update_region_canvas_coords()
        
        # Clear existing region rectangles
        for rect_id in self.rectangles:
            self.canvas.delete(rect_id)
        self.rectangles = []
        
        # Color mapping
        color_map = {
            'blue': '#0066ff',
            'red': '#ff0000',
            'green': '#00cc00',
            'yellow': '#ffcc00',
            'purple': '#9900cc',
            'orange': '#ff6600',
            'cyan': '#00cccc'
        }
        
        # Draw mask overlay if enabled
        if self.mask_enabled and self.image:
            self.draw_mask_overlay()
        
        # Draw all regions
        for name, data in self.regions.items():
            x1, y1, x2, y2 = data['canvas_coords']
            color = data.get('color', 'blue')
            color_hex = color_map.get(color, '#0066ff')
            
            # Determine visual state (check both single and multi-select)
            is_selected = name in self.selected_regions or name == self.selected_region
            is_hovered = name == self.hovered_region and self.mouse_mode == "select"
            
            # Visual feedback for selection and hover
            if is_selected:
                # Selected: thicker border, semi-transparent fill, brighter outline
                width = 4
                outline_color = color_hex
                # Add semi-transparent fill for selected region
                fill_color = self.hex_to_rgba(color_hex, 0.2)  # 20% opacity
                rect = self.canvas.create_rectangle(
                    x1, y1, x2, y2,
                    outline=outline_color, width=width, fill=fill_color,
                    tags=("region", name)
                )
                self.rectangles.append(rect)
            elif is_hovered:
                # Hovered: medium border, lighter fill
                width = 3
                outline_color = color_hex
                fill_color = self.hex_to_rgba(color_hex, 0.1)  # 10% opacity
                rect = self.canvas.create_rectangle(
                    x1, y1, x2, y2,
                    outline=outline_color, width=width, fill=fill_color,
                    tags=("region", name)
                )
                self.rectangles.append(rect)
            else:
                # Normal: thin border, no fill
                width = 2
                rect = self.canvas.create_rectangle(
                    x1, y1, x2, y2,
                    outline=color_hex, width=width, tags=("region", name)
                )
                self.rectangles.append(rect)
            
            # Add resize handles if selected (only for primary selection)
            if name == self.selected_region and len(self.selected_regions) <= 1:
                handles = self.draw_resize_handles(x1, y1, x2, y2)
                self.rectangles.extend(handles)
            
            # Add label with better visibility for selected/hovered
            label_size = 10 if (is_selected or is_hovered) else 8
            label = self.canvas.create_text(
                (x1 + x2) / 2, y1 - 10,
                text=name, fill=color_hex, font=("Arial", label_size, "bold" if is_selected else "normal"),
                anchor=tk.S, tags=("region", name)
            )
            self.rectangles.append(label)
    
    def hex_to_rgba(self, hex_color, alpha):
        """Convert hex color to rgba string for canvas fill."""
        # Remove # if present
        hex_color = hex_color.lstrip('#')
        # Convert to RGB
        r = int(hex_color[0:2], 16)
        g = int(hex_color[2:4], 16)
        b = int(hex_color[4:6], 16)
        # Return as color name or use a workaround
        # Tkinter doesn't support rgba directly, so we'll use a lighter version
        # For now, we'll create a lighter shade
        r_light = min(255, int(r + (255 - r) * alpha))
        g_light = min(255, int(g + (255 - g) * alpha))
        b_light = min(255, int(b + (255 - b) * alpha))
        return f"#{r_light:02x}{g_light:02x}{b_light:02x}"
    
    def draw_resize_handles(self, x1, y1, x2, y2):
        """Draw resize handles on selected region."""
        handles = []
        handle_size = 8
        
        # Corner handles
        corners = [
            (x1, y1),  # Top-left
            (x2, y1),  # Top-right
            (x1, y2),  # Bottom-left
            (x2, y2),  # Bottom-right
        ]
        
        # Edge handles
        edges = [
            ((x1 + x2) / 2, y1),  # Top
            ((x1 + x2) / 2, y2),  # Bottom
            (x1, (y1 + y2) / 2),  # Left
            (x2, (y1 + y2) / 2),  # Right
        ]
        
        for x, y in corners + edges:
            handle = self.canvas.create_rectangle(
                x - handle_size/2, y - handle_size/2,
                x + handle_size/2, y + handle_size/2,
                fill="white", outline="black", width=1,
                tags=("resize_handle",)
            )
            handles.append(handle)
        
        return handles
    
    def draw_mask_overlay(self):
        """Draw semi-transparent mask over areas outside selected regions."""
        if not self.image:
            return
        
        # Get canvas size (without update() to avoid blocking)
        try:
            canvas_width = self.canvas.winfo_width()
            canvas_height = self.canvas.winfo_height()
        except:
            return
        
        if canvas_width <= 1 or canvas_height <= 1:
            return
        
        # Create mask image
        mask_image = Image.new('RGBA', (canvas_width, canvas_height), (128, 128, 128, 200))
        
        # Create white rectangles for each region (holes in the mask)
        from PIL import ImageDraw
        draw = ImageDraw.Draw(mask_image)
        
        for data in self.regions.values():
            x1, y1, x2, y2 = data['canvas_coords']
            # Ensure coordinates are in order
            x_min, x_max = min(x1, x2), max(x1, x2)
            y_min, y_max = min(y1, y2), max(y1, y2)
            draw.rectangle([x_min, y_min, x_max, y_max], fill=(255, 255, 255, 0))
        
        # Convert to PhotoImage and display
        mask_photo = ImageTk.PhotoImage(mask_image)
        self.mask_overlay = self.canvas.create_image(
            canvas_width//2, canvas_height//2,
            image=mask_photo, anchor=tk.CENTER, tags="mask"
        )
        # Keep reference to prevent garbage collection
        self.canvas.mask_photo = mask_photo
    
    def update_region_list(self):
        """Update the region listbox."""
        self.region_listbox.delete(0, tk.END)
        for name in sorted(self.regions.keys()):
            self.region_listbox.insert(tk.END, name)
    
    def on_region_select(self, event):
        """Handle region selection in listbox."""
        selection = self.region_listbox.curselection()
        if not selection:
            self.selected_region = None
            self.redraw_regions()
            return
        
        region_name = self.region_listbox.get(selection[0])
        self.selected_region = region_name
        self.redraw_regions()
        
        if region_name in self.regions:
            data = self.regions[region_name]
            info = f"Region: {region_name}\n\n"
            info += f"Image Coords: {data['image_coords']}\n"
            info += f"Normalized: {data['normalized_coords']}\n"
            info += f"Color: {data.get('color', 'blue')}\n"
            info += f"Group: {data.get('group', 'None')}\n"
            
            # Calculate size
            x1, y1, x2, y2 = data['image_coords']
            width = abs(x2 - x1)
            height = abs(y2 - y1)
            info += f"Size: {width} x {height} pixels\n"
            
            if self.image:
                img_w, img_h = self.image.size
                norm_w = width / img_w
                norm_h = height / img_h
                info += f"Normalized Size: {norm_w:.3f} x {norm_h:.3f}"
            
            self.info_text.delete(1.0, tk.END)
            self.info_text.insert(1.0, info)
    
    def delete_region(self):
        """Delete selected region."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a region to delete")
            return
        
        region_name = self.region_listbox.get(selection[0])
        if region_name in self.regions:
            del self.regions[region_name]
            self.update_region_list()
            self.redraw_regions()
            self.info_text.delete(1.0, tk.END)
    
    def clear_selection(self):
        """Clear current selection."""
        if self.current_region:
            self.canvas.delete(self.current_region)
            self.current_region = None
        self.start_x = None
        self.start_y = None
    
    def export_coordinates(self):
        """Export coordinates in multiple formats (JSON, CSV, TXT, XML, YAML)."""
        if not self.regions:
            messagebox.showwarning("Warning", "No regions defined")
            return
        
        file_path = filedialog.asksaveasfilename(
            title="Export Coordinates",
            defaultextension=".json",
            filetypes=[
                ("JSON files", "*.json"),
                ("CSV files", "*.csv"),
                ("TXT files", "*.txt"),
                ("XML files", "*.xml"),
                ("YAML files", "*.yaml"),
                ("All files", "*.*")
            ]
        )
        
        if not file_path:
            return
        
        # Determine format from extension
        ext = Path(file_path).suffix.lower()
        
        try:
            if ext == '.json':
                self._export_json(file_path)
            elif ext == '.csv':
                self._export_csv(file_path)
            elif ext == '.txt':
                self._export_txt(file_path)
            elif ext == '.xml':
                self._export_xml(file_path)
            elif ext == '.yaml' or ext == '.yml':
                self._export_yaml(file_path)
            else:
                # Default to JSON
                self._export_json(file_path)
            
            messagebox.showinfo("Success", f"Coordinates exported to {file_path}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to export: {e}")
    
    def _export_json(self, file_path):
        """Export coordinates as JSON."""
        output = {
            'pdf_path': str(self.pdf_path) if self.pdf_path else None,
            'image_size': list(self.image.size) if self.image else None,
            'aspect_ratio': self.image.size[0] / self.image.size[1] if self.image else None,
            'regions': {}
        }
        
        for name, data in self.regions.items():
            # Only export normalized coordinates (percentages)
            norm_coords = data['normalized_coords']
            output['regions'][name] = {
                'normalized_coords': {
                    'x1': norm_coords[0],
                    'y1': norm_coords[1],
                    'x2': norm_coords[2],
                    'y2': norm_coords[3]
                },
                'color': data.get('color', 'blue'),
                'group': data.get('group')
            }
        
        # Add groups
        output['groups'] = self.groups
        
        with open(file_path, 'w') as f:
            json.dump(output, f, indent=2)
    
    def _export_csv(self, file_path):
        """Export coordinates as CSV."""
        import csv
        
        with open(file_path, 'w', newline='') as f:
            writer = csv.writer(f)
            # Header
            writer.writerow(['Region Name', 'Group', 'Color', 'X1 (%)', 'Y1 (%)', 'X2 (%)', 'Y2 (%)'])
            
            # Sort by name for consistency
            for name in sorted(self.regions.keys()):
                data = self.regions[name]
                norm_coords = data['normalized_coords']
                writer.writerow([
                    name,
                    data.get('group', ''),
                    data.get('color', 'blue'),
                    f"{norm_coords[0]:.6f}",
                    f"{norm_coords[1]:.6f}",
                    f"{norm_coords[2]:.6f}",
                    f"{norm_coords[3]:.6f}"
                ])
    
    def _export_txt(self, file_path):
        """Export coordinates as plain text."""
        with open(file_path, 'w') as f:
            f.write("COORDINATE CALIBRATION DATA\n")
            f.write("=" * 50 + "\n\n")
            
            if self.pdf_path:
                f.write(f"PDF Path: {self.pdf_path}\n")
            if self.image:
                f.write(f"Image Size: {self.image.size[0]} x {self.image.size[1]} pixels\n")
                f.write(f"Aspect Ratio: {self.image.size[0] / self.image.size[1]:.4f}\n")
            f.write(f"Total Regions: {len(self.regions)}\n\n")
            
            f.write("REGIONS (Normalized Coordinates - Percentages):\n")
            f.write("-" * 50 + "\n")
            
            for name in sorted(self.regions.keys()):
                data = self.regions[name]
                norm_coords = data['normalized_coords']
                f.write(f"\nRegion: {name}\n")
                f.write(f"  Group: {data.get('group', 'None')}\n")
                f.write(f"  Color: {data.get('color', 'blue')}\n")
                f.write(f"  Coordinates (normalized 0.0-1.0):\n")
                f.write(f"    X1: {norm_coords[0]:.6f} ({norm_coords[0]*100:.2f}%)\n")
                f.write(f"    Y1: {norm_coords[1]:.6f} ({norm_coords[1]*100:.2f}%)\n")
                f.write(f"    X2: {norm_coords[2]:.6f} ({norm_coords[2]*100:.2f}%)\n")
                f.write(f"    Y2: {norm_coords[3]:.6f} ({norm_coords[3]*100:.2f}%)\n")
            
            if self.groups:
                f.write("\n\nGROUPS:\n")
                f.write("-" * 50 + "\n")
                for group_name, regions in sorted(self.groups.items()):
                    f.write(f"\n{group_name} ({len(regions)} regions):\n")
                    for region in sorted(regions):
                        f.write(f"  - {region}\n")
    
    def _export_xml(self, file_path):
        """Export coordinates as XML."""
        from xml.etree.ElementTree import Element, SubElement, tostring
        from xml.dom import minidom
        
        root = Element('coordinate_calibration')
        
        if self.pdf_path:
            root.set('pdf_path', str(self.pdf_path))
        if self.image:
            root.set('image_width', str(self.image.size[0]))
            root.set('image_height', str(self.image.size[1]))
            root.set('aspect_ratio', f"{self.image.size[0] / self.image.size[1]:.6f}")
        
        regions_elem = SubElement(root, 'regions')
        for name in sorted(self.regions.keys()):
            data = self.regions[name]
            norm_coords = data['normalized_coords']
            region_elem = SubElement(regions_elem, 'region')
            region_elem.set('name', name)
            region_elem.set('group', str(data.get('group', '')))
            region_elem.set('color', data.get('color', 'blue'))
            
            coords_elem = SubElement(region_elem, 'normalized_coords')
            coords_elem.set('x1', f"{norm_coords[0]:.6f}")
            coords_elem.set('y1', f"{norm_coords[1]:.6f}")
            coords_elem.set('x2', f"{norm_coords[2]:.6f}")
            coords_elem.set('y2', f"{norm_coords[3]:.6f}")
        
        if self.groups:
            groups_elem = SubElement(root, 'groups')
            for group_name, regions in sorted(self.groups.items()):
                group_elem = SubElement(groups_elem, 'group')
                group_elem.set('name', group_name)
                for region in sorted(regions):
                    region_elem = SubElement(group_elem, 'region')
                    region_elem.text = region
        
        # Pretty print
        xml_str = minidom.parseString(tostring(root)).toprettyxml(indent="  ")
        with open(file_path, 'w') as f:
            f.write(xml_str)
    
    def _export_yaml(self, file_path):
        """Export coordinates as YAML."""
        try:
            import yaml
        except ImportError:
            messagebox.showerror("Error", "PyYAML not installed. Install with: pip install pyyaml")
            return
        
        output = {
            'pdf_path': str(self.pdf_path) if self.pdf_path else None,
            'image_size': list(self.image.size) if self.image else None,
            'aspect_ratio': self.image.size[0] / self.image.size[1] if self.image else None,
            'regions': {}
        }
        
        for name, data in self.regions.items():
            norm_coords = data['normalized_coords']
            output['regions'][name] = {
                'normalized_coords': {
                    'x1': norm_coords[0],
                    'y1': norm_coords[1],
                    'x2': norm_coords[2],
                    'y2': norm_coords[3]
                },
                'color': data.get('color', 'blue'),
                'group': data.get('group')
            }
        
        output['groups'] = self.groups
        
        with open(file_path, 'w') as f:
            yaml.dump(output, f, default_flow_style=False, sort_keys=False, indent=2)
    
    def export_mask(self):
        """Export a mask image that blacks out everything except labeled cells."""
        if not self.image or not self.regions:
            messagebox.showwarning("Warning", "Load PDF and define regions first")
            return
        
        file_path = filedialog.asksaveasfilename(
            title="Export Mask Image",
            defaultextension=".png",
            filetypes=[
                ("PNG files", "*.png"),
                ("JPEG files", "*.jpg"),
                ("TIFF files", "*.tiff"),
                ("All files", "*.*")
            ]
        )
        
        if not file_path:
            return
        
        try:
            # Create a black mask (same size as original image)
            img_width, img_height = self.image.size
            mask = Image.new('RGB', (img_width, img_height), color='black')
            
            # Draw white rectangles for each region (these are the "windows" for OCR)
            from PIL import ImageDraw
            draw = ImageDraw.Draw(mask)
            
            for name, data in self.regions.items():
                # Get image coordinates
                x1, y1, x2, y2 = data['image_coords']
                # Ensure coordinates are in order
                x_min, x_max = min(x1, x2), max(x1, x2)
                y_min, y_max = min(y1, y2), max(y1, y2)
                # Draw white rectangle (this is what OCR will see)
                draw.rectangle([x_min, y_min, x_max, y_max], fill='white', outline='white')
            
            # Save mask
            mask.save(file_path)
            
            # Also save a JSON file with the same name for reference
            json_path = Path(file_path).with_suffix('.json')
            self._export_json(str(json_path))
            
            messagebox.showinfo("Success", 
                f"Mask exported to {file_path}\n"
                f"Coordinates saved to {json_path}\n\n"
                f"The mask blacks out everything except the labeled cells.\n"
                f"Use it as an overlay during OCR to improve accuracy.")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to export mask: {e}")
    
    def load_coordinates(self):
        """Load coordinates from JSON file."""
        file_path = filedialog.askopenfilename(
            title="Load Coordinates",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if not file_path:
            return
        
        try:
            with open(file_path, 'r') as f:
                data = json.load(f)
            
            if 'regions' in data:
                self.regions = {}
                for name, region_data in data['regions'].items():
                    # Handle both old format (list) and new format (dict)
                    norm_coords_data = region_data.get('normalized_coords', [])
                    
                    # Convert dict format to list if needed
                    if isinstance(norm_coords_data, dict):
                        norm_coords = [
                            norm_coords_data.get('x1', 0.0),
                            norm_coords_data.get('y1', 0.0),
                            norm_coords_data.get('x2', 0.0),
                            norm_coords_data.get('y2', 0.0)
                        ]
                    else:
                        norm_coords = norm_coords_data
                    
                    # Calculate image coordinates from normalized if image is loaded
                    img_coords = region_data.get('image_coords', [])
                    if not img_coords and self.image and norm_coords:
                        img_w, img_h = self.image.size
                        img_coords = [
                            int(norm_coords[0] * img_w),
                            int(norm_coords[1] * img_h),
                            int(norm_coords[2] * img_w),
                            int(norm_coords[3] * img_h)
                        ]
                    
                    # Recalculate canvas coords if image is loaded
                    canvas_coords = []
                    if self.image and norm_coords:
                        img_w, img_h = self.image.size
                        canvas_coords = [
                            norm_coords[0] * img_w * self.scale_factor + self.image_x,
                            norm_coords[1] * img_h * self.scale_factor + self.image_y,
                            norm_coords[2] * img_w * self.scale_factor + self.image_x,
                            norm_coords[3] * img_h * self.scale_factor + self.image_y
                        ]
                    
                    self.regions[name] = {
                        'normalized_coords': norm_coords,
                        'image_coords': img_coords,
                        'canvas_coords': canvas_coords,
                        'color': region_data.get('color', 'blue'),
                        'group': region_data.get('group')
                    }
                
                # Load groups if present
                if 'groups' in data:
                    self.groups = data['groups']
                else:
                    # Reconstruct groups from region data
                    self.groups = {}
                    for name, region_data in data['regions'].items():
                        group = region_data.get('group')
                        if group:
                            if group not in self.groups:
                                self.groups[group] = []
                            if name not in self.groups[group]:
                                self.groups[group].append(name)
                
                self.update_region_list()
                self.update_group_list()
                self.redraw_regions()
                messagebox.showinfo("Success", f"Loaded {len(self.regions)} regions")
            else:
                messagebox.showwarning("Warning", "Invalid coordinate file format")
                
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load: {e}")
    
    def on_double_click(self, event):
        """Handle double-click to edit region name."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        clicked_region = self.get_region_at(x, y)
        if clicked_region:
            self.edit_region_name_dialog(clicked_region)
    
    def on_right_click(self, event):
        """Handle right-click for context menu."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        clicked_region = self.get_region_at(x, y)
        if clicked_region:
            self.selected_region = clicked_region
            self.redraw_regions()
            
            # Create context menu
            menu = tk.Menu(self.root, tearoff=0)
            menu.add_command(label="Edit Name", command=lambda: self.edit_region_name_dialog(clicked_region))
            menu.add_command(label="Change Color", command=lambda: self.change_region_color_dialog(clicked_region))
            menu.add_command(label="Delete", command=lambda: self.delete_region_by_name(clicked_region))
            menu.post(event.x_root, event.y_root)
    
    def on_canvas_leave(self, event):
        """Handle mouse leaving canvas - clear hover state."""
        if self.hovered_region:
            self.hovered_region = None
            self.redraw_regions()
    
    def on_mouse_move(self, event):
        """Handle mouse movement to update cursor and hover feedback."""
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        # Check if over resize handle
        handle = self.get_handle_position(x, y) if self.selected_region else None
        if handle:
            # Set cursor based on handle
            cursors = {
                'nw': 'top_left_corner', 'ne': 'top_right_corner',
                'sw': 'bottom_left_corner', 'se': 'bottom_right_corner',
                'n': 'top_side', 's': 'bottom_side',
                'w': 'left_side', 'e': 'right_side'
            }
            self.canvas.config(cursor=cursors.get(handle, 'crosshair'))
            # Clear hover when over handle
            if self.hovered_region:
                self.hovered_region = None
                self.redraw_regions()
        else:
            # Check which region is being hovered (only in select mode)
            hovered = self.get_region_at(x, y) if self.mouse_mode == "select" else None
            
            # Update hover state and redraw if changed
            if hovered != self.hovered_region:
                self.hovered_region = hovered
                self.redraw_regions()
            
            if hovered:
                self.canvas.config(cursor='fleur')  # Move cursor
            else:
                self.canvas.config(cursor='crosshair')
    
    def toggle_mask(self):
        """Toggle mask overlay."""
        self.mask_enabled = self.mask_var.get()
        self.redraw_regions()
    
    def edit_region_name_dialog(self, region_name):
        """Open dialog to edit region name."""
        if region_name not in self.regions:
            return
        
        dialog = tk.Toplevel(self.root)
        dialog.title("Edit Region Name")
        dialog.geometry("300x100")
        
        tk.Label(dialog, text="New Name:").pack(pady=10)
        entry = tk.Entry(dialog, width=30)
        entry.pack(pady=5)
        entry.insert(0, region_name)
        entry.select_range(0, tk.END)
        entry.focus()
        
        def save():
            new_name = entry.get().strip()
            if new_name and new_name != region_name:
                if new_name in self.regions:
                    messagebox.showerror("Error", f"Region '{new_name}' already exists")
                    return
                
                # Save state before renaming
                self.save_state()
                
                # Update region name
                data = self.regions.pop(region_name)
                self.regions[new_name] = data
                
                # Update groups
                for regions in self.groups.values():
                    if region_name in regions:
                        regions.remove(region_name)
                        regions.append(new_name)
                
                self.update_region_list()
                self.update_group_list()
                self.redraw_regions()
                self.selected_region = new_name
                dialog.destroy()
            elif new_name == region_name:
                dialog.destroy()
        
        tk.Button(dialog, text="Save", command=save, fg="black").pack(pady=5)
        entry.bind("<Return>", lambda e: save())
    
    def edit_region_name(self):
        """Edit name of selected region."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a region")
            return
        
        region_name = self.region_listbox.get(selection[0])
        self.edit_region_name_dialog(region_name)
    
    def change_region_color_dialog(self, region_name):
        """Open dialog to change region color."""
        if region_name not in self.regions:
            return
        
        dialog = tk.Toplevel(self.root)
        dialog.title("Change Region Color")
        dialog.geometry("250x150")
        
        tk.Label(dialog, text="Select Color:").pack(pady=10)
        color_var = tk.StringVar(value=self.regions[region_name].get('color', 'blue'))
        color_menu = ttk.Combobox(dialog, textvariable=color_var, width=15,
                                  values=["blue", "red", "green", "yellow", "purple", "orange", "cyan"])
        color_menu.pack(pady=5)
        
        def save():
            # Save state before changing color
            self.save_state()
            self.regions[region_name]['color'] = color_var.get()
            self.redraw_regions()
            dialog.destroy()
        
        tk.Button(dialog, text="Save", command=save, fg="black").pack(pady=10)
    
    def change_region_color(self):
        """Change color of selected region."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a region")
            return
        
        region_name = self.region_listbox.get(selection[0])
        self.change_region_color_dialog(region_name)
    
    def edit_selected_region(self):
        """Edit the currently selected region."""
        if not self.selected_region:
            messagebox.showwarning("Warning", "Please select a region first")
            return
        
        self.edit_region_name_dialog(self.selected_region)
    
    def delete_region_by_name(self, region_name):
        """Delete a region by name."""
        if region_name in self.regions:
            # Save state before deleting
            self.save_state()
            # Remove from groups
            for group_name, regions in list(self.groups.items()):
                if region_name in regions:
                    regions.remove(region_name)
                    if not regions:  # Remove empty groups
                        del self.groups[group_name]
            
            del self.regions[region_name]
            if self.selected_region == region_name:
                self.selected_region = None
            self.update_region_list()
            self.update_group_list()
            self.redraw_regions()
            self.info_text.delete(1.0, tk.END)
    
    def update_group_list(self):
        """Update the group listbox."""
        self.group_listbox.delete(0, tk.END)
        for group_name in sorted(self.groups.keys()):
            count = len(self.groups[group_name])
            self.group_listbox.insert(tk.END, f"{group_name} ({count} regions)")
    
    def on_group_select(self, event):
        """Handle group selection in listbox."""
        selection = self.group_listbox.curselection()
        if not selection:
            return
        
        group_text = self.group_listbox.get(selection[0])
        group_name = group_text.split(' (')[0]
        
        if group_name in self.groups:
            regions = self.groups[group_name]
            info = f"Group: {group_name}\n\n"
            info += f"Regions ({len(regions)}):\n"
            for region in sorted(regions):
                info += f"  - {region}\n"
            
            self.info_text.delete(1.0, tk.END)
            self.info_text.insert(1.0, info)
    
    def create_group(self):
        """Create a new group from selected regions."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select regions to group")
            return
        
        # Get group name
        group_name = simpledialog.askstring("Create Group", "Enter group name:")
        if not group_name:
            return
        
        if group_name in self.groups:
            if not messagebox.askyesno("Group Exists", f"Group '{group_name}' already exists. Add to it?"):
                return
        
        # Save state before grouping
        self.save_state()
        
        # Add selected regions to group
        if group_name not in self.groups:
            self.groups[group_name] = []
        
        for idx in selection:
            region_name = self.region_listbox.get(idx)
            if region_name not in self.groups[group_name]:
                self.groups[group_name].append(region_name)
            self.regions[region_name]['group'] = group_name
        
        self.update_group_list()
        self.redraw_regions()
    
    def add_to_group(self):
        """Add selected regions to a group."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select regions")
            return
        
        # Get group name
        group_name = simpledialog.askstring("Add to Group", "Enter group name:")
        if not group_name:
            return
        
        # Save state before adding to group
        self.save_state()
        
        if group_name not in self.groups:
            self.groups[group_name] = []
        
        # Add selected regions
        for idx in selection:
            region_name = self.region_listbox.get(idx)
            if region_name not in self.groups[group_name]:
                self.groups[group_name].append(region_name)
            self.regions[region_name]['group'] = group_name
        
        self.update_group_list()
        self.redraw_regions()
    
    def remove_from_group(self):
        """Remove selected regions from their group."""
        selection = self.region_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select regions")
            return
        
        # Save state before removing from group
        self.save_state()
        
        for idx in selection:
            region_name = self.region_listbox.get(idx)
            if region_name in self.regions:
                group = self.regions[region_name].get('group')
                if group and group in self.groups:
                    if region_name in self.groups[group]:
                        self.groups[group].remove(region_name)
                    if not self.groups[group]:
                        del self.groups[group]
                self.regions[region_name]['group'] = None
        
        self.update_group_list()
        self.redraw_regions()
    
    def delete_group(self):
        """Delete selected group."""
        selection = self.group_listbox.curselection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a group")
            return
        
        group_text = self.group_listbox.get(selection[0])
        group_name = group_text.split(' (')[0]
        
        if group_name in self.groups:
            # Save state before deleting group
            self.save_state()
            # Remove group from regions
            for region_name in self.groups[group_name]:
                if region_name in self.regions:
                    self.regions[region_name]['group'] = None
            
            del self.groups[group_name]
            self.update_group_list()
            self.redraw_regions()
            self.info_text.delete(1.0, tk.END)
    
    def test_extraction(self):
        """Test OCR extraction on selected regions."""
        if not self.pdf_path or not self.regions:
            messagebox.showwarning("Warning", "Load PDF and define regions first")
            return
        
        # Import OCR
        try:
            import easyocr
        except ImportError:
            messagebox.showerror("Error", "EasyOCR not installed. Install with: pip install easyocr")
            return
        
        # Test extraction for each region
        results = {}
        reader = easyocr.Reader(['en'], gpu=False)
        
        # Convert PDF to image
        images = convert_from_path(self.pdf_path, first_page=1, last_page=1)
        if not images:
            messagebox.showerror("Error", "Could not convert PDF")
            return
        
        for name, data in self.regions.items():
            # Get image coordinates
            x1, y1, x2, y2 = data['image_coords']
            
            # Crop region
            cropped = images[0].crop((x1, y1, x2, y2))
            cropped_array = np.array(cropped)
            
            # Run OCR
            ocr_results = reader.readtext(cropped_array)
            
            # Extract text
            texts = [text for (bbox, text, conf) in ocr_results if conf > 0.1]
            combined = ' '.join(texts) if texts else "(empty)"
            
            results[name] = {
                'text': combined,
                'confidence': [conf for (bbox, text, conf) in ocr_results] if ocr_results else []
            }
        
        # Display results
        result_window = tk.Toplevel(self.root)
        result_window.title("Extraction Results")
        result_window.geometry("600x400")
        
        text_widget = tk.Text(result_window, wrap=tk.WORD)
        text_widget.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        output = "OCR EXTRACTION RESULTS\n" + "="*50 + "\n\n"
        for name, result in results.items():
            output += f"{name}:\n"
            output += f"  Text: {result['text']}\n"
            if result['confidence']:
                avg_conf = sum(result['confidence']) / len(result['confidence'])
                output += f"  Avg Confidence: {avg_conf:.2%}\n"
            output += "\n"
        
        text_widget.insert(1.0, output)
        text_widget.config(state=tk.DISABLED)

def main():
    """Run the coordinate calibrator."""
    root = tk.Tk()
    CoordinateCalibrator(root)
    root.mainloop()

if __name__ == '__main__':
    main()

