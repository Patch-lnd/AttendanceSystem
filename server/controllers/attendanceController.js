// ================================
// IMPORT DE LA BASE DE DONNÉES
// ================================

// On importe la connexion MySQL
// Ce module permet d’exécuter des requêtes SQL
const db = require("../db");


// ==========================================
// GESTION DU BADGE RFID (ESP32)
// ==========================================
exports.handleAttendance = (req, res) => {

    console.log("Route POST /api/attendance atteinte");
    console.log("Données reçues :", req.body);

    // 1️⃣ Récupération de l’UID RFID envoyé par l’ESP32
    // Format attendu : { "rfid_uid": "A1B2C3D4" }
    const { rfid_uid } = req.body;

    // 2️⃣ Vérification minimale de sécurité
    // Si l’UID n’existe pas, on arrête immédiatement
    if (!rfid_uid) {
        return res.status(400).json({
            status: "error",
            message: "UID RFID manquant"
        });
    }

    // 3️⃣ Recherche de l’utilisateur correspondant à l’UID
    const findUserQuery = "SELECT * FROM users WHERE rfid_uid = ?";

    db.query(findUserQuery, [rfid_uid], (err, results) => {

        // 4️⃣ Erreur SQL
        if (err) {
            console.error(err);
            return res.status(500).json({
                status: "error",
                message: "Erreur serveur lors de la recherche utilisateur"
            });
        }

        // 5️⃣ Carte inconnue
        if (results.length === 0) {
            return res.status(404).json({
                status: "error",
                message: "Utilisateur inexistant"
            });
        }

        // 6️⃣ Utilisateur trouvé
        const user = results[0];

        // 7️⃣ Inversion de l’état de présence
        // true → false | false → true
        const newPresenceStatus = !user.is_present;

        // 8️⃣ Mise à jour dans la base de données
        const updateQuery = "UPDATE users SET is_present = ? WHERE id = ?";

        db.query(updateQuery, [newPresenceStatus, user.id], (updateErr) => {

            // 9️⃣ Erreur lors de la mise à jour
            if (updateErr) {
                console.error(updateErr);
                return res.status(500).json({
                    status: "error",
                    message: "Erreur lors de la mise à jour de la présence"
                });
            }

            // 🔟 SOCKET.IO
            // On récupère l’instance io stockée dans app.js
            const io = req.app.get("io");

            // On notifie TOUS les dashboards connectés
            io.emit("attendanceUpdate", {
                id: user.id,
                full_name: user.full_name,
                rfid_uid: user.rfid_uid,
                is_present: newPresenceStatus
            });

            // 1️⃣1️⃣ Réponse HTTP finale envoyée à l’ESP32
            return res.status(200).json({
                status: "success",
                user: {
                    id: user.id,
                    full_name: user.full_name,
                    is_present: newPresenceStatus
                }
            });
        });
    });
};


// ==========================================
// AFFICHAGE DU DASHBOARD (NAVIGATEUR)
// ==========================================
exports.renderDashboard = (req, res) => {

    // Requête SQL pour récupérer tous les utilisateurs
    const query = "SELECT * FROM users";

    db.query(query, (err, results) => {

        // Gestion d’erreur base de données
        if (err) {
            console.error(err);
            return res
                .status(500)
                .send("Erreur lors du chargement du dashboard");
        }

        // Rendu de la vue dashboard.ejs
        res.render("dashboard", {
            users: results
        });
    });
};
