// On importe Express uniquement pour utiliser son système de routes
const express = require("express");

// On crée un routeur Express
// Ce routeur va contenir les routes liées à la présence
const router = express.Router();

// On importe le controller
// C’est lui qui contient la logique métier
const attendanceController = require("../controllers/attendanceController");

// Définition de la route POST /api/attendance
// Quand une requête POST arrive sur cette URL,
// la fonction handleAttendance est exécutée
router.post("/attendance", attendanceController.handleAttendance);

// On exporte le routeur
// Pour pouvoir l’utiliser dans app.js

module.exports = router;
