const express = require("express");

// On importe CORS pour autoriser les requêtes externes (ESP32, navigateur)
const cors = require("cors");

// On charge les variables d’environnement depuis le fichier .env
require("dotenv").config();

// On crée l’application Express
const app = express();

// =====================
// MIDDLEWARES
// =====================

// Autorise les requêtes venant d’autres appareils (ESP32, frontend) CORS: Cross-Origin Resource Sharing
app.use(cors());

// Permet de lire le JSON envoyé dans les requêtes POST
app.use(express.json());

// Permet de lire les données envoyées par formulaire (optionnel ici)
app.use(express.urlencoded({ extended: true }));

// =====================
// ROUTES
// =====================

// Importation des routes de présence
const attendanceRoutes = require("./routes/attendanceRoutes");

// Toutes les routes définies dans attendanceRoutes
// seront accessibles via /api/...
app.use("/api", attendanceRoutes);

// =====================
// ROUTE DE TEST
// =====================

// Route simple pour vérifier que le serveur fonctionne
app.get("/", (req, res) => {
    res.send("Attendance System Server is running");
});

// =====================
// LANCEMENT DU SERVEUR
// =====================

// Port défini dans .env ou 3000 par défaut
const PORT = process.env.PORT;

// Démarrage du serveur
app.listen(PORT, () => {
    console.log(`Serveur lancé sur le port ${PORT}`);
});
