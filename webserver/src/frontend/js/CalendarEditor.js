const w = 800;
const h = 600;

const secondsInDay = 24*60*60;

const weekdayIndexToWeekday = {
    0 : "Monday",
    1 : "Tuesday",
    2 : "Wednesday",
    3 : "Thursday",
    4 : "Friday",
    5 : "Saturday",
    6 : "Sunday"
}

const weekdayToWeekdayIndex = {
    "Monday"    : 0,
    "Tuesday"   : 1,
    "Wednesday" : 2,
    "Thursday"  : 3,
    "Friday"    : 4,
    "Saturday"  : 5,
    "Sunday"    : 6
}


class timeStamp {
    constructor(second, minute, hour, weekday = 0, day = 0, month = 0){
        this.second = second;
        this.minute = minute;
        this.hour = hour;

        this.weekday = weekday;

        this.day = day;
        this.month = month;
    }

    getTime(){
        return (this.second + 60 *
            (this.minute + 60 *
                (this.hour + 24 *
                    (this.weekday + this.day + 12 *
                        (this.month)))))
    }

    toHMSFormat(){
        let hourField = this.hour
        if (hourField < 10){
            hourField = "0" + hourField
        }

        let minuteField = this.minute
        if (minuteField < 10){
            minuteField = "0" + minuteField
        }

        let secondField = this.second
        if (secondField < 10){
            secondField = "0" + secondField
        }

        return (hourField + ":" + minuteField + ":" + secondField)
    }
}


function createTimeStamp(inSeconds, scheduleType){
    switch (scheduleType){
        case 1: //daily
            console.log("Daily schedule not implemented")
            break
        case 2: //weekly

            break
        case 3: //monthly
            console.log("Monthly schedule not implemented")
            break
        case 4: //yearly
            console.log("Yearly schedule not implemented")
            break
        default:
            console.log("Invalid schedule type")
            return
    }

    if(scheduleType === 2){ //weekly
        let newTimeStamp = new timeStamp(0, 0, 0, 0, 0, 0)
        let weekday = floor(inSeconds/(24*60*60))
        let dayTime = inSeconds%(24*60*60)

        let hour = floor(dayTime/(60*60))
        let hourTime = dayTime%(60*60)

        let minutes = floor(hourTime/(60))
        let seconds = hourTime%60

        newTimeStamp.weekday = weekday
        newTimeStamp.hour = hour
        newTimeStamp.minute = minutes
        newTimeStamp.second = seconds

        return newTimeStamp
    }
}

class Clickable{
    constructor(ref, x, y, w, h){
        this.ref = ref;
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
    }

    click(){
        this.ref.click();
    }
}


//click handling
var clickables = []
//clickables implement .click()

var selected

//My html elements
const scheduleFieldsToggle = document.getElementById("scheduleFieldsToggle")

const usernameElement = document.getElementById("username")
const scheduleIDElement = document.getElementById("scheduleId")
const scheduleJSONElement = document.getElementById("scheduleJSON")

const scheduleTitle = document.getElementById("scheduleTitle")
const scheduleDescription = document.getElementById("scheduleDescription")

const scheduleTitleRow = document.getElementById("scheduleTitleRow")
const scheduleDescriptionRow = document.getElementById("scheduleDescriptionRow")

const eventName = document.getElementById("eventName")
const startTime = document.getElementById("startTime")
const startWeekday = document.getElementById("startWeekday")
const stopTime = document.getElementById("stopTime")
const stopWeekday = document.getElementById("stopWeekday")
const duration = document.getElementById("duration")
const progName = document.getElementById("progName")
const extraArgs = document.getElementById("extraArgs")

const addOrEditButton = document.getElementById("addEditEvent");

const selectedLabel = document.getElementById("selectedName")

function resetFormFields(){
    eventName.value = ""
    startTime.value = "00:00:00"
    startWeekday.value = "Monday"
    stopTime.value = "00:00:00"
    stopWeekday.value = "Monday"
    duration.value = ""
    progName.value = ""
    extraArgs.value = ""
}

function mouseClicked(){
    //reject mouse clicks from beyond
    if(mouseX < 0 || mouseX >= w || mouseY < 0 || mouseY > h){
        return
    }

    let foundAny = false;

    for (let i = 0; i < clickables.length; i++){
        let clickable = clickables[i];

        if(mouseX >= clickable.x && mouseX < clickable.x + clickable.w && mouseY >= clickable.y && mouseY < clickable.y + clickable.h){
            clickable.click();
            foundAny = true;
            selected = clickable.ref

            addOrEditButton.value = "Edit event"

            selected.fillFormVals()
        }
    }

    if(!foundAny){
        selected = null

        addOrEditButton.value = "Add event"

        resetFormFields()
    }
}



const eventColors = [[149, 175, 186], [189, 196, 167], [213, 225, 163], [63, 124, 172], [226, 248, 156]]
let currColor = 0

function complementaryColor(c){
    if(c.length === 1){
        c = 255 - c;
    }
    else if(c.length === 3){
        c[0] = 255 - c[0];
        c[1] = 255 - c[1];
        c[2] = 255 - c[2];
    }

    return c
}

class Event {
    //name string
    //startTime && endTime are timeStamps
    //program string
    //args string array
    constructor(name, startTime, stopTime, duration, program, args){
        this.name = name;
        this.startTime = startTime;
        this.stopTime = stopTime;
        this.duration = duration;
        this.program = program;
        this.args = args;
        this.color = eventColors[currColor]
        currColor = (currColor+1)%eventColors.length
        //clickable property
        this.selected = false;
    }

    click(){
        selectedLabel.innerText = this.name
    }

    fillFormVals(){
        eventName.value = this.name

        startTime.value = this.startTime.toHMSFormat()
        startWeekday.value = weekdayIndexToWeekday[this.startTime.weekday]


        stopTime.value = this.stopTime.toHMSFormat()
        stopWeekday.value = weekdayIndexToWeekday[this.stopTime.weekday]

        duration.value = this.duration
        progName.value = this.program
        extraArgs.value = this.args
    }
}



var ParallelEvents = [
    //[capacity, current]
    [0, 0], //monday
    [0, 0], //tuesday
    [0, 0], //wednesday
    [0, 0], //thursday
    [0, 0], //friday
    [0, 0], //saturday
    [0, 0]  //sunday
]

var events = []

function setup() {

    createCanvas(w, h).position(windowWidth - w - 50, 50);

    LoadSchedule(scheduleJSONElement.value)

}

function drawWeekGrid(){
    fill(color('white'))
    rect(0, 0, w/8, h)
    textSize(16)

    text("Hour\\Day", w/64, h/25 - 5)


    for(let j = 1; j < 25; j++){ //hour boundaries
        fill(color('white'))
        rect(0, j * h/25, w/8, h/25)
        fill(color('black'))
        text((j-1) + ":00", w/32, (1 + j) * h/25 - 5)
    }


    for(let i = 1; i < 8; i++){ //day separators
        fill(color('white'))
        rect(i * w/8, 0, w/8, h)
        fill(color('black'))
        text(weekdayIndexToWeekday[i-1], i*w/8 + w/16 - 5 * weekdayIndexToWeekday[i-1].length, h/25 - 5)

        fill(color('white'))

        for(let j = 1; j < 25; j++){ //hour boundaries
            rect(i * w/8, j * h/25, w/8, h/25)

        }
    }

}

function drawEventWeekly(event){
    //in pixels
    var totalLen = ((event.stopTime.getTime() - event.startTime.getTime())/secondsInDay) * (24 * h/25);
    //in pixels
    var start = ((event.startTime.getTime() % secondsInDay)/secondsInDay) * (24*h/25);
    var dayInc = 0;


    while(totalLen > 0){
        var dayWidth = w/8;
        var eventWidth = dayWidth/ParallelEvents[event.startTime.weekday + dayInc][0];
        var eventWidthOff = eventWidth * ParallelEvents[event.startTime.weekday + dayInc][1];
        ParallelEvents[event.startTime.weekday + dayInc][1]++;

        var clipLen = 0;
        if(totalLen < ((24*h/25) - start)){
            clipLen = totalLen;
        }
        else{
            clipLen = (24*h/25) - start;
        }

        var tabX = (event.startTime.weekday + dayInc) * dayWidth + dayWidth + eventWidthOff;
        var tabY = w/25 + start - 8;
        var tabW = eventWidth - 1;
        var tabH = clipLen;

        //event tab
        if(event === selected){
            fill(color("white"))
        }
        else{
            fill(color(event.color))
        }
        rect(tabX, tabY, tabW, tabH)


        var clickable = new Clickable(
            event,
            tabX,
            tabY,
            tabW,
            tabH
        )
        clickables = concat(clickables, clickable)

        dayInc++;
        start = 0
        totalLen -= clipLen;
    }
}

function addToParallelEventArray(event){
    var totalPeriod = (event.stopTime.getTime() - event.startTime.getTime());
    var start = ((event.startTime.getTime()) % secondsInDay);

    var dayInc = 0;

    while (totalPeriod > 0){
        var clipPeriod = 0;
        if (totalPeriod < (secondsInDay - start)){
            clipPeriod = totalPeriod;
        }
        else{
            clipPeriod = secondsInDay - start;
        }

        ParallelEvents[event.startTime.weekday + dayInc][0]++;

        totalPeriod -= clipPeriod;
        dayInc++;
        start = 0;
    }
}

//weekly
function draw() {
    background(255)

    clickables = []

    ParallelEvents = [
        //[capacity, current]
        [0, 0], //monday
        [0, 0], //tuesday
        [0, 0], //wednesday
        [0, 0], //thursday
        [0, 0], //friday
        [0, 0], //saturday
        [0, 0]  //sunday
    ]

    for(let i = 0; i < events.length; i ++){
        addToParallelEventArray(events[i])
    }

    fill(color('white'))
    textAlign(LEFT, BOTTOM)
    drawWeekGrid();
    for(let i = 0; i < events.length; i ++){
        drawEventWeekly(events[i])
    }

    textAlign(CENTER, CENTER)
    for (let i = 0; i < clickables.length; i++) {
        let clickable = clickables[i];
        let event = clickable.ref

        if(event === selected){
            fill(color("black"))
        }
        else{
            var tmp = []
            tmp = concat(tmp, event.color)
            fill(color(complementaryColor(tmp)))
        }

        push()
        translate(
            clickable.x + clickable.w/2,
            clickable.y + clickable.h/2
        )
        rotate(3*PI/2)
        text(event.name,
            0,
            0
        )
        pop()
    }
}


function AddEditButton(){
    console.log(addOrEditButton.value + " clicked!")

    if(eventName.value === "" ||
        startTime.value === "" ||
        stopTime.value === "" ||
        startWeekday.value === "" ||
        stopWeekday.value === "" ||
        (startTime.value >= stopTime.value && weekdayToWeekdayIndex[startWeekday.value] >= weekdayToWeekdayIndex[stopWeekday.value]) ||
        duration.value === "" ||
        progName.value === ""
    ){ //invalid parameters
        return
    }

    //edit the selected one
    if(selected != null){
        selected.name = eventName.value

        let timeFields = startTime.value.split(':')
        selected.startTime = new timeStamp(
            parseInt(timeFields[2]),
            parseInt(timeFields[1]),
            parseInt(timeFields[0]),
            weekdayToWeekdayIndex[startWeekday.value])


        timeFields = stopTime.value.split(':')
        selected.stopTime = new timeStamp(
            parseInt(timeFields[2]),
            parseInt(timeFields[1]),
            parseInt(timeFields[0]),
            weekdayToWeekdayIndex[stopWeekday.value])


        selected.duration = duration.value
        selected.program = progName.value
        //smth with currArg?

        selected.args = extraArgs.value

    }
    else{ //actually add one
        console.log("adding event")

        let startTimeFields = startTime.value.split(':')
        let stopTimeFields = stopTime.value.split(':')

        let startTimeStamp = new timeStamp(
            parseInt(startTimeFields[2]),
            parseInt(startTimeFields[1]),
            parseInt(startTimeFields[0]),
            weekdayToWeekdayIndex[startWeekday.value]
        )

        let stopTimeStamp = new timeStamp(
            parseInt(stopTimeFields[2]),
            parseInt(stopTimeFields[1]),
            parseInt(stopTimeFields[0]),
            weekdayToWeekdayIndex[stopWeekday.value]
        )


        let newEvent = new Event(
            "" + eventName.value,
            startTimeStamp,
            stopTimeStamp,
            "" + duration.value,
            "" + progName.value,
            "" + extraArgs.value
        )

        events.push(newEvent)

        selected = newEvent
    }
}

function RemoveButton(){
    if(selected == null){
        return
    }

    events = events.filter(function (event){
        return event !== selected
    })

    selected = null
}

let ScheduleEvents = {}
function LoadSchedule(scheduleJSON){
    let schedule = JSON.parse(scheduleJSON)
    scheduleTitle.value = schedule.header.title
    scheduleDescription.value = schedule.header.description

    let scheduleType = schedule.header.scheduleType;
    let entries = schedule.entries;

    let currTime = 0
    if (entries !== null){
        for(let i = 0; i < entries.length; i++){
            let entry = entries[i]

            if(entry.entryType === 1){ //time set
                currTime = entry.time
            }
            else if (entry.entryType === 2){ //open event
                let entryProgName = entry.args[0]
                entry.args.shift()
                let entryArgs = [].concat(entry.args)

                let newEvent = new Event(
                    entryProgName,
                    createTimeStamp(currTime, scheduleType),
                    null,
                    entry.duration,
                    entryProgName,
                    entryArgs,
                )

                ScheduleEvents[entry.id] = newEvent
                events.push(newEvent)
            }
            else if (entry.entryType === 3){ //close event
                ScheduleEvents[entry.id].stopTime = createTimeStamp(currTime, scheduleType)
            }

        }
    }


}

function SaveScheduleButton(){
    //build up a schedule from the present events
    let schedule = {
        "header": {
            "title": scheduleTitle.value,
            "description": scheduleDescription.value,
            "scheduleType": 2 //weekly
        },
        "entries": []
    }

    let scheduleWrapper = {
        "name"  : scheduleTitle.value,
        "id"    : parseInt(scheduleIDElement.value, 10),
        "owner" : usernameElement.value,
        "jsonSchedule" : schedule
    }

    let startTimeEventSort = [].concat(events)
    startTimeEventSort.sort((event1, event2) => {
        return event1.startTime.getTime() - event2.startTime.getTime()
    })

    let stopTimeEventSort = [].concat(events)
    stopTimeEventSort.sort((event1, event2) => {
        return event1.stopTime.getTime() - event2.startTime.getTime()
    })

    let eventTimes = []

    for(let i = 0; i < events.length; i++){
        let _event = events[i];
        let _startTimeStamp = _event.startTime.getTime()
        if(!eventTimes.includes(_startTimeStamp)){
            eventTimes.push(_startTimeStamp)
        }

        let _stopTimeStamp = _event.stopTime.getTime()
        if(!eventTimes.includes(_stopTimeStamp)){
            eventTimes.push(_stopTimeStamp)
        }
    }
    eventTimes.sort((time1, time2) => {
        return time1 - time2
    })


    let currTime = -1;
    let id = 1
    //for each event
    let idList = {}
    for (let i = 0; i < eventTimes.length; i++){
        let eventTime = eventTimes[i]
        //automatically sets time
        if(currTime < eventTime){
            currTime = eventTime

            let timeUpdate = {
                "entryType" : 1,
                "time" : currTime
            }
            schedule.entries = concat(schedule.entries, timeUpdate);
        }

        for(let i = 0; i < startTimeEventSort.length; i++){
            //check if we create one or destroy one
            if(startTimeEventSort[i].startTime.getTime() === currTime){
                let event = startTimeEventSort[i]

                schedule.entries = schedule.entries.concat({
                    "entryType" : 2,
                    "id" : id,
                    "duration" : parseInt(event.duration),
                    "args" : [ //TODO fix
                        event.program,
                    ].concat(event.args)
                })

                idList[startTimeEventSort[i]] = id //e.g. idList[event1] = 1
                id++;
            }

            if(stopTimeEventSort[i].stopTime.getTime() === currTime){
                schedule.entries = schedule.entries.concat({
                    "entryType" : 3,
                    "id" : idList[stopTimeEventSort[i]]
                })
            }
        }
    }

    console.log(scheduleWrapper)
    const scheduleJSON = JSON.stringify(scheduleWrapper);

    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/modifySchedule");
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(scheduleJSON);
}

function HideShowScheduleFields(){
    if(scheduleFieldsToggle.value === "Show schedule fields"){ //unhide them
        scheduleTitleRow.style.display = "block"
        scheduleDescriptionRow.style.display = "block"

        scheduleFieldsToggle.value = "Hide schedule fields"
    }
    else{
        scheduleTitleRow.style.display = "none"
        scheduleDescriptionRow.style.display = "none"

        scheduleFieldsToggle.value = "Show schedule fields"
    }
}