// ===============================
// CONNEXION AU SERVEUR SOCKET.IO
// ===============================

// On se connecte automatiquement au serveur
// (même domaine que Node.js)
const socket = io();

// ===============================
// RÉCEPTION DES MISES À JOUR
// ===============================

socket.on("attendanceUpdate", (data) => {

    /*
        data contient :
        {
            id,
            full_name,
            rfid_uid,
            is_present
        }
    */

    console.log("Mise à jour reçue :", data);

    // 1️⃣ On récupère la carte correspondante
    const card = document.getElementById(`user-${data.id}`);

    // Si la carte n’existe pas (sécurité)
    if (!card) return;

    // 2️⃣ Texte du statut
    const statusText = card.querySelector(".status-text");

    // 3️⃣ Nettoyage des anciennes couleurs
    card.classList.remove(
        "bg-green-600/40",
        "ring-green-400/40",
        "bg-red-600/40",
        "ring-red-400/40"
    );

    // 4️⃣ Application du nouvel état
    if (data.is_present) {

        card.classList.add(
            "bg-green-600/40",
            "ring-1",
            "ring-green-400/40"
        );

        statusText.textContent = "PRÉSENT";

    } else {

        card.classList.add(
            "bg-red-600/40",
            "ring-1",
            "ring-red-400/40"
        );

        statusText.textContent = "ABSENT";
    }

    // 5️⃣ Animation vibration (feedback visuel)
    card.animate(
        [
            { transform: "translateX(0)" },
            { transform: "translateX(-4px)" },
            { transform: "translateX(4px)" },
            { transform: "translateX(0)" }
        ],
        {
            duration: 300,
            easing: "ease-in-out"
        }
    );
});
