const express = require("express");
const bodyParser = require("body-parser");
const cors = require("cors");
const path = require("path");
require("dotenv").config();
const mongoose = require("mongoose");

// Import MongoDB-related logic and API route from data.js
const { saveData, getData } = require("./data");

const app = express();

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, "../public")));

// Serve the frontend
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "../public", "index.html"));
});

// API route for handling POST request to save data
app.post("/api/data", saveData);

// API route for handling GET request to retrieve data
app.get("/api/data", getData);

// Start the server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
