let sizeFactor    = 0.95;
let minChartWidth = 0;
let maxChartWidth = 99999999;

const timeTable = {
    "Unfiltered" : 0,
    "5m"         : 300,
    "30m"        : 1800,
    "1h"         : 3600,
    "3h"         : 10800,
    "6h"         : 21600,
    "12h"        : 43200,
    "1d"         : 86400,
    "7d"         : 604800
}

const plotArrangements = {
    "Landscape" : {
        "factor"   : 0.95,
        "classList": ["container-landscape"],
        "minWidth" : 0,
        "maxWidth" : 99999999
    },
    "Grid": {
        "factor"   : 0.35,
        "classList": ["container-grid"],
        "minWidth" : 300,
        "maxWidth" : 500,
    }
}

function createChartById(id) {
    return MyCharts.createChart(document.getElementById(id), {
        width: window.innerWidth*sizeFactor, 
        height: 300,
        layout: {
            background: { color: '#121212' }, // chart background
            textColor: '#D1D4DC',             // axis + labels
        },
        grid: {
            vertLines: { color: '#2B2B43' },
            horzLines: { color: '#2B2B43' },
        },
        timeScale: {
            timeVisible: true,     // show HH:MM on the x-axis
            secondsVisible: false, // set to true if you want seconds
        },
    });
}

function changeTimeFrame(data, period) {
    if (period == 0) return data;
    const result = new Array();
    const cumulative = new Array();
    const treshold = 1.25;
    let resolution = 0;
    let begin = 0;
    data.forEach((entry, idx) => {
        if (idx != 0) resolution = entry[0] - data[idx - 1][0];
        if (begin == 0) {
            begin = entry[0]
        }
        cumulative.push(entry);
        if (entry[0] - begin + resolution*treshold>= period || idx == data.length - 1) {
            console.log(entry[0])
            console.log(begin)
            console.log(resolution)
            let max = 0;
            let min = 9999999;
            cumulative.forEach(entry2 => {
                if (entry2[3] > max) max = entry2[3];
                if (entry2[4] < min) min = entry2[4];
            });
            result.push([
                cumulative[cumulative.length - 1][0], // timestamp
                cumulative[0][1],                     // open
                cumulative[cumulative.length - 1][2], // close
                max,                                  // high
                min,                                  // low
                cumulative[cumulative.length - 1][5], // temp
                cumulative[cumulative.length - 1][6], // humid
                cumulative[cumulative.length - 1][7], // bmp
                cumulative[cumulative.length - 1][8], // battery
                cumulative[cumulative.length - 1][9], // windir
            ]);
            cumulative.length = 0;
            begin = 0;
        }

    });
    return result;
}

function fetchData() {
    const timeframeValue = document.getElementById("timeframe").value;
    const period = timeTable[timeframeValue];

    fetch("/data", {
        method: "GET",
        headers: {
            'Content-Type': 'application/json'
        },
    })
    .then(response => response.json())
    .then(data => {
        const windSpeedData     = new Array();
        const windDirectionData = new Array();
        const temperatureData   = new Array();
        const humidityData      = new Array();
        const bmpData           = new Array();
        const batteryData       = new Array();
        let dataArray = Array.from(data);
        dataArray = changeTimeFrame(dataArray, period);
        dataArray.forEach(entry => {
            windSpeedData.push({time: entry[0], open: entry[1], close: entry[2], high: entry[3], low: entry[4]});
            windDirectionData.push({time: entry[0], value: (entry[9]-1)/8*360});
            temperatureData.push  ({time: entry[0], value: entry[5]});
            humidityData.push     ({time: entry[0], value: entry[6]});
            bmpData.push          ({time: entry[0], value: entry[7]});
            batteryData.push      ({time: entry[0], value: entry[8]});
        });
        windSpeedSeries.setData    (windSpeedData);
        windDirectionSeries.setData(windDirectionData);
        temperatureSeries.setData  (temperatureData);
        bmpSeries.setData          (bmpData);
        humiditySeries.setData     (humidityData);
        batterySeries.setData      (batteryData);
    });
}

const windSpeedChart     = createChartById('wind-speed-chart');
const windDirectionChart = createChartById('wind-direction-chart');
const temperatureChart   = createChartById('temperature-chart');
const humidityChart      = createChartById('humidity-chart');
const bmpChart           = createChartById('bmp-chart');
const batteryChart       = createChartById('battery-chart');

const windSpeedSeries     = windSpeedChart.addSeries     (MyCharts.CandlestickSeries);
const windDirectionSeries = windDirectionChart.addSeries (MyCharts.LineSeries);
const temperatureSeries   = temperatureChart.addSeries   (MyCharts.LineSeries);
const humiditySeries      = humidityChart.addSeries      (MyCharts.LineSeries);
const bmpSeries           = bmpChart.addSeries           (MyCharts.LineSeries);
const batterySeries       = batteryChart.addSeries       (MyCharts.LineSeries);

function changeArrangement() {
    const selector      = document.getElementById("plot-arrangement");
    const container     = document.getElementById("container");
    const arrangement   = plotArrangements[selector.value];
    container.classList = arrangement["classList"];
    sizeFactor          = arrangement["factor"];
    minChartWidth       = arrangement["minWidth"];
    maxChartWidth       = arrangement["maxWidth"];
    window.dispatchEvent(new Event('resize'));
}

fetchData();

setInterval(() => {
    fetch("/data-changed").then(data => {
        data.text().then(res => {
            if (res == "yes") fetchData();
        });
    });
}, 1000);

document.getElementById("timeframe").addEventListener("input", () => {
    fetchData();
});

document.getElementById("plot-arrangement").addEventListener("input", () => {
    changeArrangement();
});

window.addEventListener("resize", () => {
    windSpeedChart.applyOptions     ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
    windDirectionChart.applyOptions ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
    temperatureChart.applyOptions   ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
    humidityChart.applyOptions      ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
    bmpChart.applyOptions           ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
    batteryChart.applyOptions       ({ width: Math.min(Math.max(window.innerWidth*sizeFactor, minChartWidth), maxChartWidth) });
});