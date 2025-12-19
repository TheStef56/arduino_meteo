let wSizeFactor           = 0.95;
let maxChartWidth         = 99999999;
let minChartWidth         = 0;
let hSizeFactor           = 0.3;
let maxChartHeight        = 99999999;
let minChartHeight        = 300;
let darkMode              = true;
let currentTimezoneOffset = 3600;
let currentArrangement    = "Landscape";
let localData             = [];
let baselineSelected      = [];
let candleSelected        = [];
let meanSelected          = [];

const plotArrangements = {
    "Landscape" : {
        "wFactor"   : 0.95,
        "classList": ["container-landscape"],
        "minWidth" : 0,
        "maxWidth" : 99999999
    },
    "Grid": {
        "wFactor"   : 0.35,
        "classList": ["container-grid"],
        "minWidth" : 150,
        "maxWidth" : 500,
    }
}

const timeTable = {
    "Unfiltered" : 300,
    "5m"         : 300,
    "30m"        : 1800,
    "1h"         : 3600,
    "3h"         : 10800,
    "6h"         : 21600,
    "12h"        : 43200,
    "1d"         : 86400,
    "7d"         : 604800
}

const timeZones = {
    "UTC-12 (BIT)": -12, 
    "UTC-11 (SST)": -11, 
    "UTC-10 (HST)": -10, 
    "UTC-9 (AKST)": -9, 
    "UTC-8 (PST)": -8, 
    "UTC-7 (MST)": -7, 
    "UTC-6 (CST)": -6, 
    "UTC-5 (EST)": -5, 
    "UTC-4 (AST)": -4, 
    "UTC-3 (ART)": -3, 
    "UTC-2 (GST)": -2, 
    "UTC-1 (AZOT)": -1, 
    "UTC-0 (GMT)": -0, 
    "UTC+1 (CET)": +1, 
    "UTC+2 (EET)": +2, 
    "UTC+3 (MSK)": +3, 
    "UTC+4 (GST)": +4, 
    "UTC+5 (PKT)": +5, 
    "UTC+6 (BST)": +6, 
    "UTC+7 (ICT)": +7, 
    "UTC+8 (CST)": +8, 
    "UTC+9 (JST)": +9, 
    "UTC+10 (AEST)": +10, 
    "UTC+11 (SBT)": +11, 
    "UTC+12 (NZST)": +12, 
    "UTC+13 (TOT)": +13, 
    "UTC+14 (LINT)": +14, 
}

function createChartById(id) {
    return MyCharts.createChart(document.getElementById(id), {
        width: window.innerWidth*wSizeFactor, 
        height: window.innerHeight*hSizeFactor,
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

function changeTimezone(data) {
    data.forEach((_, idx) => {
        // main timestamp
        data[idx][0] = data[idx][0] + currentTimezoneOffset;
        
        // candles timestamps
        data[idx][10][0] = data[idx][10][0] + currentTimezoneOffset;
        data[idx][11][0] = data[idx][11][0] + currentTimezoneOffset;
        data[idx][12][0] = data[idx][12][0] + currentTimezoneOffset;
        data[idx][13][0] = data[idx][13][0] + currentTimezoneOffset;
        data[idx][14][0] = data[idx][14][0] + currentTimezoneOffset;
    })
}

function changeTimeFrame(data, period) {
    // if (period == 0) return data;
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
            let maxWind    = 0;
            let maxTemp    = 0;
            let maxHumid   = 0;
            let maxBmp     = 0;
            let maxBattery = 0;
            let maxWindir  = 0;
            let minWind    = 9999999;
            let minTemp    = 9999999;
            let minHumid   = 9999999;
            let minBmp     = 9999999;
            let minBattery = 9999999;
            let minWindir  = 9999999;
            cumulative.forEach(entry2 => {
                if (entry2[3] > maxWind) maxWind = entry2[3];
                if (entry2[4] < minWind) minWind = entry2[4];

                if (entry2[6]  > maxTemp)    maxTemp    = entry2[6];
                if (entry2[7]  > maxHumid)   maxHumid   = entry2[7];
                if (entry2[8]  > maxBmp)     maxBmp     = entry2[8];
                if (entry2[9]  > maxBattery) maxBattery = entry2[9];
                if (entry2[10] > maxWindir)  maxWindir  = entry2[10];

                if (entry2[6]  < minTemp)    minTemp    = entry2[6];
                if (entry2[7]  < minHumid)   minHumid   = entry2[7];
                if (entry2[8]  < minBmp)     minBmp     = entry2[8];
                if (entry2[9]  < minBattery) minBattery = entry2[9];
                if (entry2[10] < minWindir)  minWindir  = entry2[10];


            });
            const cml = cumulative.length;
            const end = cumulative.length - 1;
            result.push([
                cumulative[end][0],  // timestamp
                cumulative[0][1],    // open
                cumulative[end][2],  // close
                maxWind,             // high
                minWind,             // low
                cumulative[end][6],  // temp
                cumulative[end][7],  // humid
                cumulative[end][8],  // bmp
                cumulative[end][9],  // battery
                cumulative[end][10], // windir
                [cumulative[end][0], cml > 1 ? cumulative[0][6]  : data[data.length != 1 ? idx-1 : 0][6],  cumulative[end][6],  maxTemp,    minTemp,],    // timestamp, open, close, max, min (temp)
                [cumulative[end][0], cml > 1 ? cumulative[0][7]  : data[data.length != 1 ? idx-1 : 0][7],  cumulative[end][7],  maxHumid,   minHumid,],   // timestamp, open, close, max, min (humid)
                [cumulative[end][0], cml > 1 ? cumulative[0][8]  : data[data.length != 1 ? idx-1 : 0][8],  cumulative[end][8],  maxBmp,     minBmp,],     // timestamp, open, close, max, min (BMP)
                [cumulative[end][0], cml > 1 ? cumulative[0][9]  : data[data.length != 1 ? idx-1 : 0][9],  cumulative[end][9],  maxBattery, minBattery,], // timestamp, open, close, max, min (battery)
                [cumulative[end][0], cml > 1 ? cumulative[0][10] : data[data.length != 1 ? idx-1 : 0][10], cumulative[end][10], maxWindir,  minWindir,],  // timestamp, open, close, max, min (windir)
                cumulative[end][5],  // mean
            ]);
            cumulative.length = 0;
            begin = 0;
        }

    });
    return result;
}

function updateCharts() {
    allCandleSeries.forEach((_, idx) => {
        if (candleSelected.includes(idx)) {
            updateSingleChart(idx, "candles", true);
        } else {
            updateSingleChart(idx, "candles", false);
        }
    });

    allSeries.forEach((_, idx) => {
        if (baselineSelected.includes(idx)) {
            updateSingleChart(idx, "baseline", true);
        } else {
            updateSingleChart(idx, "baseline", false);
        }
    });

    meanSeries.forEach((_, idx) => {
        if (meanSelected.includes(idx)) {
            updateSingleChart(idx, "mean", true);
        } else {
            updateSingleChart(idx, "mean", false);
        }
    });
}

function initSelectedCharts() {
    Array.from(document.getElementsByClassName("chart-type-icon-button")).forEach(iconButton => {
        let chartId = 0;
        let type = "";
        let value = false;
        const target = iconButton;
        if (target.getAttribute("chart-id")) {
                chartId = parseInt(target.getAttribute("chart-id"));
        } else {
            chartId = parseInt(target.parentElement.getAttribute("chart-id"));
        }
        if (target.classList.contains("baseline") || target.parentElement.classList.contains("baseline")) {
            type = "baseline";
        } else if (target.classList.contains("candles") || target.parentElement.classList.contains("candles")){
            type = "candles";
        } else if (target.classList.contains("mean") || target.parentElement.classList.contains("mean")) {
            type = "mean";
        }

        if (target.classList.contains("active") || target.parentElement.classList.contains("active")) {
            value = true;
        } else {
            value = false;
        }
        
        if (value) {
            if (type == "candles" && !(candleSelected.includes(chartId))) {
                candleSelected.push(chartId);
            } else if (type == "baseline" && !(baselineSelected.includes(chartId))) {
                baselineSelected.push(chartId);
            } else if (type == "mean"  && !meanSelected.includes(chartId)) {
                meanSelected.push(chartId);
            }
        }
    })
}

function updateSingleChart(chartId, type, value=true) {
    const timeframeValue = document.getElementById("timeframe").innerText.trim();
    const period = timeTable[timeframeValue];
    let dataArray = Array.from(localData);
    dataArray = changeTimeFrame(dataArray, period);
    changeTimezone(dataArray);

    if (type == "baseline") {
        if (value) {
            if (chartId == 0) {
                allSeries[0].setData(dataArray.map((entry) => {return {time: entry[0], value: entry[2]}}));
            } else {
                allSeries[chartId].setData(dataArray.map((entry) => {return {time: entry[0], value: entry[chartId+4]}}));
            }
        } else {
            allSeries[chartId].setData([]);
        }
    } else if (type == "candles") {
        if (value) {
            if (chartId == 0) {
                windSpeedSeries.setData(dataArray.map((entry) => {return {time: entry[0], open: entry[1], close: entry[2], high: entry[3], low: entry[4]}}));
            } else {
                allCandleSeries[chartId].setData(dataArray.map((entry) => {return {time: entry[9+chartId][0], open: entry[9+chartId][1], close: entry[9+chartId][2], high: entry[9+chartId][3], low: entry[9+chartId][4]}}));
            }
        } else {
            if (chartId == 0) {
                windSpeedSeries.setData([]);
            } else {
                allCandleSeries[chartId].setData([]);
            }
        }
    } else if (type == "mean") {
        if (value) {
            meanSeries[chartId].setData(dataArray.map((entry) => {return {time: entry[0], value: entry[15+chartId]}}));
        } else {
            meanSeries[chartId].setData([]);
        }
    }
}

function fetchData() {
    fetch("/data", {
        method: "GET",
        headers: {
            'Content-Type': 'application/json'
        },
    })
    .then(response => response.json())
    .then(data => {
        localData = data;
        updateCharts();
    });
}

const windSpeedChart     = createChartById('wind-speed-chart');
const windDirectionChart = createChartById('wind-direction-chart');
const temperatureChart   = createChartById('temperature-chart');
const humidityChart      = createChartById('humidity-chart');
const bmpChart           = createChartById('bmp-chart');
const batteryChart       = createChartById('battery-chart');

const allCharts = [windSpeedChart, windDirectionChart, temperatureChart, humidityChart, bmpChart, batteryChart]

const windSpeedSeries         = windSpeedChart.addSeries    (MyCharts.CandlestickSeries);
const windDirectionSeries     = windDirectionChart.addSeries(MyCharts.CandlestickSeries);
const temperatureSeries       = temperatureChart.addSeries  (MyCharts.CandlestickSeries);
const humiditySeries          = humidityChart.addSeries     (MyCharts.CandlestickSeries);
const bmpSeries               = bmpChart.addSeries          (MyCharts.CandlestickSeries);
const batterySeries           = batteryChart.addSeries      (MyCharts.CandlestickSeries);

const allCandleSeries = [windSpeedSeries, temperatureSeries, humiditySeries, bmpSeries, batterySeries, windDirectionSeries]

const windBaselineSpeedSeries     = windSpeedChart.addSeries    (MyCharts.LineSeries);
const windDirectionBaselineSeries = windDirectionChart.addSeries(MyCharts.LineSeries);
const temperatureBaselineSeries   = temperatureChart.addSeries  (MyCharts.LineSeries);
const humidityBaselineSeries      = humidityChart.addSeries     (MyCharts.LineSeries);
const bmpBaselineSeries           = bmpChart.addSeries          (MyCharts.LineSeries);
const batteryBaselineSeries       = batteryChart.addSeries      (MyCharts.LineSeries);

const allSeries = [windBaselineSpeedSeries, temperatureBaselineSeries, humidityBaselineSeries, bmpBaselineSeries, batteryBaselineSeries, windDirectionBaselineSeries]

const windMeanSpeedSeries = windSpeedChart.addSeries(MyCharts.LineSeries, {color: "#00e426"});

const meanSeries = [windMeanSpeedSeries];

// UI callbacks ---------------------------------------------------------------------------

function changeArrangement() {
    const selector            = document.getElementById("arrangement");
    const container           = document.getElementById("container");
    const arrangement         = plotArrangements[selector.innerText];
    currentArrangement        = selector.innerText;
    container.classList       = arrangement["classList"];
    wSizeFactor               = arrangement["wFactor"];
    minChartWidth             = arrangement["minWidth"];
    maxChartWidth             = arrangement["maxWidth"];
    window.dispatchEvent(new Event('resize'));
}

function changeArrangementLabel(label) {
    document.getElementById("arrangement").innerText = label;
    changeArrangement();
}

function changeTimeFrameLabel(label) {
    document.getElementById("timeframe").innerText = label;
    updateCharts();
}

function changeTimezoneLabel(label) {
    document.getElementById("timezone").innerText = label;
    currentTimezoneOffset = timeZones[label]*3600;
    updateCharts();
}

function changeThemeLabel(label) {
    const element = document.getElementById("theme");
    if (label == "Light") {
        document.documentElement.style.setProperty('--custom-background', 'var(--light-white)');
        document.documentElement.style.setProperty('--custom-foreground', 'var(--dark-white)');
        document.documentElement.style.setProperty('--black-white', 'black');
        Array.from(document.getElementsByClassName("text-white")).forEach(element => {
            element.classList.remove('text-white');
            element.classList.add('text-black');
        })
        allCharts.forEach(chart => {
            chart.applyOptions({
                layout: {
                    background: { color: '#FFFFFF' }, // chart background
                    textColor: '#000000',             // axis + labels
                },
                grid: {
                    vertLines: { color: '#D6DCDE' },
                    horzLines: { color: '#D6DCDE' },
                }});
        });
        element.innerText = "Light";
    } else if (label == "Dark") {
        document.documentElement.style.setProperty('--custom-background', 'var(--light-grey)');
        document.documentElement.style.setProperty('--custom-foreground', 'var(--dark-black)');
        document.documentElement.style.setProperty('--black-white', 'white');
        Array.from(document.getElementsByClassName("text-black")).forEach(element => {
            element.classList.remove('text-black');
            element.classList.add('text-white');
        })
        allCharts.forEach(chart => {
            chart.applyOptions({
                layout: {
                    background: { color: '#121212' }, // chart background
                    textColor: '#D1D4DC',             // axis + labels
                },
                grid: {
                    vertLines: { color: '#2B2B43' },
                    horzLines: { color: '#2B2B43' },
                }});
        });
        element.innerText = "Dark";
    }
}

function expandChart(target) {
    if (target.classList.contains("chart-expanded")) {
        target.classList.remove("chart-expanded");
        target.classList.add("chart-container");
        hSizeFactor = 0.3;
        wSizeFactor = plotArrangements[currentArrangement]["wFactor"];
        maxChartWidth = plotArrangements[currentArrangement]["maxWidth"];
    } else {
        target.classList.remove("chart-container");
        target.classList.add("chart-expanded");
        hSizeFactor = 0.97;
        wSizeFactor = 1;
        maxChartWidth = 99999999999;
    }
    window.dispatchEvent(new Event('resize'));
}

//  Navbar stuff ---------------------------------------------------------------------------

document.addEventListener("DOMContentLoaded", () => {
    // Get all "navbar-burger" elements
    const $navbarBurgers = Array.prototype.slice.call(document.querySelectorAll(".navbar-burger"), 0);

    // Add a click event on each of them
    $navbarBurgers.forEach( el => {
        el.addEventListener("click", () => {
            // Get the target from the "data-target" attribute
            const target = el.dataset.target;
            const $target = document.getElementById(target);

            // Toggle the "is-active" class on both the "navbar-burger" and the "navbar-menu"
            el.classList.toggle("is-active");
            $target.classList.toggle("is-active");
            if (el.classList.contains("is-active")) {
                document.getElementById("navbarBasicExample").classList.add("scrollable-navbar");
            } else {
                document.getElementById("navbarBasicExample").classList.remove("scrollable-navbar");
            }
        });
    });

    // resetting media screen navbar state to avoid bug -----------------------------------------------

    window.addEventListener("resize", () => {
        if (window.innerWidth >= 1431) {
            $navbarBurgers.forEach( el => {
                const target = el.dataset.target;
                const $target = document.getElementById(target);
                el.classList.remove("is-active");
                $target.classList.remove("is-active");
                document.getElementById("navbarBasicExample").classList.remove("scrollable-navbar");
            });
        }
    });
    let lastTap = 0;
    Array.from(document.querySelectorAll("canvas")).forEach(canvas => {
        const callback = (ev) => {
            const target = ev.target.parentElement.parentElement.parentElement.parentElement.parentElement.parentElement;
            expandChart(target);
        }
        canvas.addEventListener("dblclick", callback);
        canvas.addEventListener("touchend", (evnt) => {
            const now = Date.now();
            const timeDiff = now - lastTap;

            if (timeDiff < 300 && timeDiff > 0) {
                callback(evnt)
            }

            if (evnt.touches.length == 0) lastTap = now;
        });
    });

    // chart type selection logic --------------------------------------------------------------------------

    Array.from(document.getElementsByClassName("chart-type-icon-button")).forEach(iconButton => {
        iconButton.addEventListener("click", (ev) => {
            let chartId = 0;
            let type = "";
            let value = false;
            const target = ev.target.parentElement;
            if (target.getAttribute("chart-id")) {
                    chartId = parseInt(target.getAttribute("chart-id"));
            } else {
                chartId = parseInt(target.parentElement.getAttribute("chart-id"));
            }
            if (target.classList.contains("baseline") || target.parentElement.classList.contains("baseline")) {
                type = "baseline";
            } else if (target.classList.contains("candles") || target.parentElement.classList.contains("candles")){
                type = "candles";
            } else if (target.classList.contains("mean") || target.parentElement.classList.contains("mean")) {
                type = "mean";
            }

            if (target.classList.contains("active") || target.parentElement.classList.contains("active")) {
                target.classList.remove("active");
                target.parentElement.classList.remove("active");
                value = false;
            } else {
                target.classList.add("active");
                target.parentElement.classList.add("active");
                value = true;
            }

            updateSingleChart(chartId, type, value);
            if (value) {
                if (type == "candles" && !(candleSelected.includes(chartId))) {
                    candleSelected.push(chartId);
                } else if (type == "baseline" && !(baselineSelected.includes(chartId))) {
                    baselineSelected.push(chartId);
                } else if (type == "mean"  && !meanSelected.includes(chartId)) {
                    meanSelected.push(chartId);
                }
            } else {
                if (type == "candles" && candleSelected.includes(chartId)) {
                    candleSelected.splice(candleSelected.indexOf(chartId), 1);
                } else if (type == "baseline" && baselineSelected.includes(chartId)) {
                    baselineSelected.splice(baselineSelected.indexOf(chartId), 1);
                } else if (type == "mean" && meanSelected.includes(chartId)) {
                    meanSelected.splice(meanSelected.indexOf(chartId), 1);
                }
            }
        })
    })
    // initialization -----------------------------------------------------------------

    initSelectedCharts();
    fetchData();
    setInterval(() => {
        fetch("/data-changed").then(data => {
            data.text().then(res => {
                if (res == "yes") fetchData();
            });
        });
    }, 5000);

    window.addEventListener("resize", () => {
        allCharts.forEach(chart => {
            chart.applyOptions({ 
                width : Math.min(Math.max(window.innerWidth*wSizeFactor , minChartWidth ), maxChartWidth),
                height: Math.min(Math.max(window.innerHeight*hSizeFactor, minChartHeight), maxChartHeight)
            });
        });    
    });
});

