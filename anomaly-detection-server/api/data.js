require("dotenv").config();
const mongoose = require("mongoose");

const uri = `mongodb+srv://${process.env.DB_USER}:${process.env.DB_PASSWORD}@${process.env.CLUSTER_URL}/${process.env.DB_NAME}?retryWrites=true&w=majority&appName=${process.env.APP_NAME}`;

let isConnected = false;

async function connectToDatabase() {
  if (isConnected) {
    return;
  }
  try {
    await mongoose.connect(uri, {
      useNewUrlParser: true,
      useUnifiedTopology: true,
    });
    isConnected = true;
    console.log("Connected to MongoDB Atlas");
  } catch (err) {
    console.error("Error connecting to MongoDB Atlas:", err);
    throw err;
  }
}

// Define schema and model for storing data
const DataSchema = new mongoose.Schema({
  meterId: String,
  time: Date,
  anomalousPowerReading: Number,
});
const DataModel = mongoose.model("Data", DataSchema);

// Save data to MongoDB
async function saveData(req, res) {
  await connectToDatabase();
  const { meterId, time, anomalousPowerReading } = req.body;
  try {
    const newData = new DataModel({ meterId, time, anomalousPowerReading });
    await newData.save();
    res.status(201).json({ message: "Data saved successfully!" });
  } catch (err) {
    res.status(500).json({ error: "Error saving data" });
  }
}

// Retrieve data from MongoDB
async function getData(req, res) {
  await connectToDatabase();
  try {
    const data = await DataModel.find();
    res.status(200).json(data);
  } catch (err) {
    res.status(500).json({ error: "Error retrieving data" });
  }
}

module.exports = { saveData, getData };
