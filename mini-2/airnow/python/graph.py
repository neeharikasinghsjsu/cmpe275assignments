import pandas as pd
import plotly.express as px

df = pd.read_csv('data.csv', header=None, names=['Time', 'Pollutant', 'Concentration', 'Unit', 'Raw Concentration', 'AQI', 'Category'])

df['Time'] = pd.to_datetime(df['Time'], unit='us')

def get_color_and_quality(aqi):
    if aqi >= 0 and aqi <= 50:
        return 'Good', 'Green'
    elif aqi <= 100:
        return 'Moderate', 'Yellow'
    elif aqi <= 150:
        return 'Unhealthy for Sensitive Groups', 'Orange'
    elif aqi <= 200:
        return 'Unhealthy', 'Red'
    elif aqi <= 300:
        return 'Very Unhealthy', 'Purple'
    else:
        return 'Hazardous', 'Maroon'

df['Quality'], df['Color'] = zip(*df['AQI'].apply(get_color_and_quality))

color_map = {'Good': 'Green', 'Moderate': 'Yellow', 'Unhealthy for Sensitive Groups': 'Orange', 'Unhealthy': 'Red', 'Very Unhealthy': 'Purple', 'Hazardous': 'Maroon'}

fig = px.bar(df, x='Time', y='AQI', color='Quality', color_discrete_map=color_map, hover_data=['Pollutant', 'Quality', 'Concentration', 'Unit'], title='Air Quality Index')
fig.update_layout(xaxis_title='Time', yaxis_title='AQI', coloraxis_colorbar=dict(title='Quality'))
fig.show()
