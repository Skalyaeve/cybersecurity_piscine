const sql = require('mssql')
const express = require('express')
const bodyParser = require('body-parser')

const config = {
        user: 'SA',
        password: '<MyStr0ngP@ssword!>',
        server: 'localhost',
        database: 'test',
        options: { encrypt: false }
}
sql.connect(config, err => {
        if (err) throw err
        console.log('Backend connected to Microsoft SQL database.')
})

const app = express()
app.use(bodyParser.json())
app.post('/login', async (req, res) => {
        const { username, password } = req.body
        try {
                const request = new sql.Request()
                const result = await request.query("SELECT * FROM users WHERE username = '" + username + "' AND password = '" + password + "'")
                res.json({ success: result.recordset.length > 0, result: result.recordset })
        }
        catch (err) { res.status(500).json({ success: false, message: err.message }) }
})
app.get('/login', async (req, res) => {
        const { username, password } = req.query
        try {
                const request = new sql.Request()
                const result = await request.query("SELECT * FROM users WHERE username = '" + username + "' AND password = '" + password + "'")
                res.json({ success: result.recordset.length > 0, result: result.recordset })
        }
        catch (err) { res.status(500).json({ success: false, message: err.message }) }
})
app.listen(8585, () => { console.log(' Microsoft SQL backend listen on port 8585.') })
