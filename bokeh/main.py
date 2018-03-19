import numpy as np
from bokeh.io import curdoc
from bokeh.layouts import row, widgetbox, column, layout
from bokeh.models import ColumnDataSource, Legend
from bokeh.models.callbacks import CustomJS
from bokeh.models.widgets import Slider, TextInput, Button, TextInput, Div
from bokeh.plotting import figure
from bokeh.util import session_id
from bokeh.models import SingleIntervalTicker, LinearAxis
from bokeh.resources import CDN
from bokeh.embed import file_html


import socket
import sys

time_arr = np.array([0])
x_arr = np.array([0])
goal_arr = np.array([-80])
act_arr = np.array([0])
halt_arr = np.array([0])
iter_arr = np.array([0])
ret_arr = np.array([0])
source = ColumnDataSource(data=dict(time=time_arr, x=x_arr, act=act_arr, halt=halt_arr, goal=goal_arr),name='src')
retSource = ColumnDataSource(data=dict(iter=iter_arr, ret=ret_arr),name='retsrc')

x_plot = figure(plot_height=150, plot_width=500, title="Car Position",tools="", toolbar_location=None,
                y_range=[-100, 120])
x_plot.line('time', 'x', source=source, line_width=3, line_alpha=0.6, line_join='round', legend = 'Position')
x_plot.line('time', 'goal', source=source, line_width=1, line_alpha=1, line_join='round', legend = 'Goal', line_color='green')
x_plot.legend.orientation = "horizontal"
x_plot.legend.location = (220,80)
x_plot.legend.border_line_width = 0
x_plot.legend.border_line_color = None
x_plot.legend.background_fill_alpha = 0.5

act_plot = figure(plot_height=100, plot_width=500, title="Action (Force command)",tools="", toolbar_location=None,
                  y_range=[-0.1, 1.1], y_axis_type=None)
act_plot.line('time', 'act', source=source, line_width=3, line_alpha=0.6, line_join='round')
act_ticker = SingleIntervalTicker(interval=1, num_minor_ticks=0)
act_yaxis = LinearAxis(ticker=act_ticker)
act_plot.add_layout(act_yaxis, 'left')

halt_plot = figure(plot_height=100, plot_width=500, title="Goal Reached (Reset)",tools="", toolbar_location=None,
                   y_range=[-0.1, 1.1], y_axis_type=None)
halt_plot.line('time', 'halt', source=source, line_width=3, line_alpha=0.6, line_join='round')
halt_ticker = SingleIntervalTicker(interval=1, num_minor_ticks=0)
halt_yaxis = LinearAxis(ticker=halt_ticker)
halt_plot.add_layout(halt_yaxis, 'left')

ret_plot = figure(plot_height=200, plot_width=500, title="Episode Return",tools = "pan,wheel_zoom,box_zoom,reset",active_drag='box_zoom', active_scroll=None)

ret_plot.line('iter', 'ret', source=retSource, line_width=3, line_alpha=0.6, line_join='round')


#callback = CustomJS(args=dict(source=source, retsource=retSource), code="""
callback = CustomJS(code="""
""")
start_callback = CustomJS(code="""
  var code = encodeURI(editor.getValue())
  code = code.replace(/#/g, '%23');
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open( "GET", "http://cps.ics.uci.edu/getcode.php?" + code, false ); // false for synchronous request
  xmlHttp.send( null );
""")
ql_callback = CustomJS(code="""
  $(function(){ $.get("http://cps.ics.uci.edu/static/rlalg.c", function(data){editor.setValue(data);});});
""")
bt = Button(
    label="Refersh",width=1,
    button_type="success",
    callback=callback
)

ql_bt = Button(
    label="Q-learning",width=140,
    button_type="success",
    callback=ql_callback
)
hand_bt = Button(
    label="Hand-Engineered",width=140,
    button_type="success",
    callback=callback
)
single_bt = Button(
    label="Single Pass",width=140,
    button_type="success",
    callback=callback
)
reset_bt = Button(
    label="Reset",width=140,
    button_type="success",
    callback=callback
)
start_bt = Button(
    label="Start",width=140,
    button_type="success",
    callback=start_callback
)
stop_bt = Button(
    label="Stop",width=140,
    button_type="success",
    callback=callback
)


edit_div = Div(text="""
    <div id="editor", width=600 ></div>
""", id="editor", width=650,
)
yt_div = Div(text="""
<iframe width="500" height="300" src="https://www.youtube.com/embed/72AxgCTk8Hc?autoplay=1&origin=http://cps.ics.uci.edu:5006" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
""", width=500,height=300)

html = file_html(row(column(row(ql_bt, hand_bt, single_bt, reset_bt),edit_div,row(start_bt, stop_bt)),column(yt_div, x_plot, act_plot,halt_plot, ret_plot)), CDN, "OPEB")
print html

# curdoc().add_root(row(column(row(ql_bt, hand_bt, single_bt, reset_bt),edit_div,row(start_bt, stop_bt)),column(yt_div, x_plot, act_plot, halt_plot, ret_plot)))
# curdoc().title = "OPEB"

