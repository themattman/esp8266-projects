d3.csv("sensor_sample.txt")
    .then(makeChart);

var myLabs;
var myVals;

function makeChart(datum) {
    var dataLabels = datum.map(function(d) {return d.datetime});
    var dataValues = datum.map(function(d) {return d.temp1F});
    var dataValues2 = datum.map(function(d) {return d.humidity});

    var ctx = document.getElementById('myChart').getContext('2d');
    var chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: dataLabels,
            datasets: [{
                label: 'TempF',
                // backgroundColor: 'rgb(255, 99, 132)',
                borderColor: 'rgb(255, 99, 132)',
                data: dataValues,
            },
            {
                label: 'Humidity',
                // backgroundColor: 'rgb(255, 99, 132)',
                // borderColor: 'rgb(255, 99, 132)',
                data: dataValues2,
            }]
        },
        options: {}
    });

    myLabs = dataLabels;
    myVals = dataValues;
}
