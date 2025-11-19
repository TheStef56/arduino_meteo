let wSizeFactor           = 0.95;
let maxChartWidth         = 99999999;
let minChartWidth         = 0;
let hSizeFactor           = 0.3;
let maxChartHeight        = 99999999;
let minChartHeight        = 300;
let darkMode              = true;
let currentTimezoneOffset = 3600;
let currentArrangement    = "Landscape";

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
        data[idx][0] = data[idx][0] + currentTimezoneOffset;
    })
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
    const timeframeValue = document.getElementById("timeframe").innerText;
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
        changeTimezone(dataArray);
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

const allCharts = [ windSpeedChart, windDirectionChart, temperatureChart, humidityChart, bmpChart, batteryChart]

const windSpeedSeries     = windSpeedChart.addSeries     (MyCharts.CandlestickSeries);
const windDirectionSeries = windDirectionChart.addSeries (MyCharts.LineSeries);
const temperatureSeries   = temperatureChart.addSeries   (MyCharts.LineSeries);
const humiditySeries      = humidityChart.addSeries      (MyCharts.LineSeries);
const bmpSeries           = bmpChart.addSeries           (MyCharts.LineSeries);
const batterySeries       = batteryChart.addSeries       (MyCharts.LineSeries);

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
    fetchData();
}

function changeTimezoneLabel(label) {
    document.getElementById("timezone").innerText = label;
    currentTimezoneOffset = timeZones[label]*3600;
    fetchData();
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

    // resetting media screen navbar state to avoid bug

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
});