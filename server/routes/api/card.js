const express = require('express')
const router = express.Router()
const cardController = require('../../controllers/attendanceControllers')

// Route to verifiy UID of card
router.post('/card', cardController.checkCard);
module.exports = router;