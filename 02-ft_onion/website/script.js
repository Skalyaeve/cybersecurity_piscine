const canvas = document.createElement('canvas')
const ctx = canvas.getContext('2d')
document.body.appendChild(canvas)

canvas.width = window.innerWidth
canvas.height = window.innerHeight

const playerRadius = 20
let playerX = 150
let playerY = canvas.height / 2
let playerSpeed = 5
let playerSpeedInc = 0.005

const gravity = -0.5
const playerJumpPower = 10
let playerYvel = 0

let score = 0
let obstacles = []

const obstacleInterval = 100
let obstacleCounter = 0
let gameOver = false

const scoreText = document.createElement('div')
scoreText.style.position = 'absolute'
scoreText.style.top = '30px'
scoreText.style.left = '30px'
scoreText.style.color = 'white'
scoreText.style.fontFamily = 'Arial'
scoreText.style.fontSize = '60px'
document.body.appendChild(scoreText)

document.addEventListener('keydown', (event) => {
        if (event.code === 'Space') playerYvel = playerJumpPower
})

function createObstacle() {
        const obstacleWidth = 50
        const obstacleHeight = Math.random() * (canvas.height / 2 - 100) + 100
        const obstacleX = canvas.width
        const obstacleY = Math.random() < 0.5 ? 0 : canvas.height - obstacleHeight
        obstacles.push({
                x: obstacleX,
                y: obstacleY,
                width: obstacleWidth,
                height: obstacleHeight,
                passed: false,
        })
}

function update() {
        ctx.clearRect(0, 0, canvas.width, canvas.height)

        ctx.beginPath()
        ctx.arc(playerX, playerY, playerRadius, 0, Math.PI * 2)
        ctx.fillStyle = 'white'
        ctx.fill()
        ctx.closePath()

        obstacles.forEach(obstacle => {
                if (gameOver) return

                ctx.beginPath()
                ctx.rect(obstacle.x, obstacle.y, obstacle.width, obstacle.height)
                ctx.fillStyle = 'white'
                ctx.fill()
                ctx.closePath()
                if (
                        playerX + playerRadius > obstacle.x &&
                        playerX < obstacle.x + obstacle.width
                ) {
                        if (
                                (obstacle.y < canvas.height / 2 && playerY < obstacle.y + obstacle.height) ||
                                (obstacle.y > canvas.height / 2 && playerY + playerRadius > obstacle.y)
                        ) {
                                window.alert("Well done! Score: " + score)
                                window.location.reload()
                                gameOver = true
                        }
                }
                if (obstacle.x + obstacle.width < playerX && !obstacle.passed) {
                        score++
                        obstacle.passed = true
                        console.log(score)
                }

                obstacle.x -= playerSpeed
                if (obstacle.x < 0 - obstacle.width - 1000)
                        obstacles.splice(obstacles.indexOf(obstacle), 1)
        })
        if (gameOver) return

        playerYvel += gravity
        playerY -= playerYvel
        playerSpeed += playerSpeedInc

        obstacleCounter += playerSpeed / 5
        if (obstacleCounter >= obstacleInterval) {
                createObstacle()
                obstacleCounter = 0
        }

        scoreText.textContent = score
        window.requestAnimationFrame(update)
}

update()
