let height
let width
let board
let scale

function setup() {
    height = frameElement.getAttribute("mheight") //32
    width = frameElement.getAttribute("mwidth") //64
    scale = frameElement.getAttribute("scale") //12
    matrix = frameElement.getAttribute("matrix")
    board = matrix.split(" ")



    if(height < 0 || width < 0 || scale < 0){
        console.log("Wrong dimensions provided\n")
        return
    }

    createCanvas(width*scale, height*scale);

    background(0, 0, 0);

    for(let i = 0; i < height; i++){
        for(let j = 0; j < width; j++){
            if(i * 64 + j < board.length)
                fill(board[i*64 +j])
            else
                fill(0) //should be 0
            rect(j*scale, i*scale, scale, scale)
        }
    }
}