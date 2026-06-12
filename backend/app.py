from datetime import datetime, date
from typing import Optional

import csv
import io

from fastapi import FastAPI, Header, HTTPException, Query
from fastapi.responses import StreamingResponse
from pydantic import BaseModel
from sqlalchemy import (
    create_engine, Column, Integer, String, Float, Boolean,
    DateTime, Date, ForeignKey, Text
)
from sqlalchemy.orm import declarative_base, sessionmaker, relationship, Session


# ================= CONFIG =================

DATABASE_URL = "sqlite:///./plantation.db"
ESP_TOKEN = "CHANGE_THIS_SECRET_TOKEN"

engine = create_engine(
    DATABASE_URL,
    connect_args={"check_same_thread": False}
)

SessionLocal = sessionmaker(bind=engine, autoflush=False, autocommit=False)
Base = declarative_base()

app = FastAPI(title="Indoor Plantation API")


# ================= MODELS =================

class PlantDevice(Base):
    __tablename__ = "plant_devices"

    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(String(100), unique=True, index=True, nullable=False)
    device_name = Column(String(150), nullable=True)
    location = Column(String(150), nullable=True)
    plant_type = Column(String(100), nullable=True)
    seed_date = Column(Date, nullable=True)

    firmware_version = Column(String(50), nullable=True)
    hardware_version = Column(String(50), nullable=True)

    is_online = Column(Boolean, default=False)
    last_seen = Column(DateTime, nullable=True)

    status = Column(Boolean, default=True)
    created_at = Column(DateTime, default=datetime.utcnow)

    sensor_logs = relationship("PlantSensorLog", back_populates="device")
    event_logs = relationship("PlantEventLog", back_populates="device")
    growth_logs = relationship("PlantGrowthLog", back_populates="device")


class PlantSensorLog(Base):
    __tablename__ = "plant_sensor_logs"

    id = Column(Integer, primary_key=True, index=True)
    device_id_fk = Column(Integer, ForeignKey("plant_devices.id"))

    temperature_c = Column(Float, nullable=True)
    humidity_percent = Column(Float, nullable=True)

    soil1_raw = Column(Integer, nullable=True)
    soil2_raw = Column(Integer, nullable=True)
    soil1_percent = Column(Float, nullable=True)
    soil2_percent = Column(Float, nullable=True)

    water_raw = Column(Integer, nullable=True)
    water_percent = Column(Float, nullable=True)

    co2_ppm = Column(Integer, nullable=True)

    grow_light_power = Column(Integer, default=0)
    grow_light_pwm = Column(Integer, nullable=True)

    wifi_signal = Column(Integer, nullable=True)
    uptime_seconds = Column(Integer, nullable=True)

    seed_day = Column(Integer, nullable=True)
    growth_stage = Column(String(100), nullable=True)

    free_heap = Column(Integer, nullable=True)
    chip_temperature = Column(Float, nullable=True)

    created_at_device = Column(DateTime, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)

    device = relationship("PlantDevice", back_populates="sensor_logs")


class PlantEventLog(Base):
    __tablename__ = "plant_event_logs"

    id = Column(Integer, primary_key=True, index=True)
    device_id_fk = Column(Integer, ForeignKey("plant_devices.id"))

    event_type = Column(String(100), nullable=False)
    event_value = Column(Float, nullable=True)
    severity = Column(String(50), default="info")
    description = Column(Text, nullable=True)

    wifi_signal = Column(Integer, nullable=True)
    uptime_seconds = Column(Integer, nullable=True)

    created_at_device = Column(DateTime, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)

    device = relationship("PlantDevice", back_populates="event_logs")


class PlantGrowthLog(Base):
    __tablename__ = "plant_growth_logs"

    id = Column(Integer, primary_key=True, index=True)
    device_id_fk = Column(Integer, ForeignKey("plant_devices.id"))

    log_date = Column(Date, nullable=False)
    plant_height_cm = Column(Float, nullable=True)
    leaf_count = Column(Integer, nullable=True)
    leaf_color = Column(String(100), nullable=True)
    health_score = Column(Integer, nullable=True)
    growth_stage = Column(String(100), nullable=True)
    notes = Column(Text, nullable=True)

    watering_done = Column(Boolean, default=False)
    fertilizer_applied = Column(Boolean, default=False)
    disease_detected = Column(Boolean, default=False)

    created_at = Column(DateTime, default=datetime.utcnow)

    device = relationship("PlantDevice", back_populates="growth_logs")



class PlantProfile(Base):
    __tablename__ = "plant_profiles"

    id = Column(Integer, primary_key=True, index=True)

    plant_name = Column(String(100), unique=True)

    soil_moisture_min = Column(Float)
    soil_moisture_max = Column(Float)

    temp_min_c = Column(Float)
    temp_max_c = Column(Float)

    humidity_min = Column(Float)
    humidity_max = Column(Float)

    light_hours = Column(Float)

    led_pwm = Column(Integer)
    lumens = Column(Integer)

    led_height_cm = Column(Float)

    growth_days_min = Column(Integer)
    growth_days_max = Column(Integer)

Base.metadata.create_all(bind=engine)
# ================= SCHEMAS =================

class DeviceCreate(BaseModel):
    device_id: str
    device_name: Optional[str] = None
    location: Optional[str] = None
    plant_type: Optional[str] = None
    seed_date: Optional[date] = None
    firmware_version: Optional[str] = None
    hardware_version: Optional[str] = None


class SensorLogCreate(BaseModel):
    device_id: str

    temperature_c: Optional[float] = None
    humidity_percent: Optional[float] = None

    soil1_raw: Optional[int] = None
    soil2_raw: Optional[int] = None
    soil1_percent: Optional[float] = None
    soil2_percent: Optional[float] = None

    water_raw: Optional[int] = None
    water_percent: Optional[float] = None

    co2_ppm: Optional[int] = None

    grow_light_power: Optional[int] = 0
    grow_light_pwm: Optional[int] = None

    wifi_signal: Optional[int] = None
    uptime_seconds: Optional[int] = None

    seed_day: Optional[int] = None
    growth_stage: Optional[str] = None

    free_heap: Optional[int] = None
    chip_temperature: Optional[float] = None

    created_at_device: Optional[datetime] = None


class EventLogCreate(BaseModel):
    device_id: str
    event_type: str
    event_value: Optional[float] = None
    severity: Optional[str] = "info"
    description: Optional[str] = None
    wifi_signal: Optional[int] = None
    uptime_seconds: Optional[int] = None
    created_at_device: Optional[datetime] = None


class GrowthLogCreate(BaseModel):
    device_id: str
    log_date: date
    plant_height_cm: Optional[float] = None
    leaf_count: Optional[int] = None
    leaf_color: Optional[str] = None
    health_score: Optional[int] = None
    growth_stage: Optional[str] = None
    notes: Optional[str] = None
    watering_done: Optional[bool] = False
    fertilizer_applied: Optional[bool] = False
    disease_detected: Optional[bool] = False


class PlantProfileCreate(BaseModel):
    plant_name: str

    soil_moisture_min: float
    soil_moisture_max: float

    temp_min_c: float
    temp_max_c: float

    humidity_min: float
    humidity_max: float

    light_hours: float

    led_pwm: int
    lumens: int

    led_height_cm: float

    growth_days_min: int
    growth_days_max: int


# ================= HELPERS =================

def get_db():
    db = SessionLocal()
    try:
        return db
    finally:
        pass


def close_db(db: Session):
    db.close()


def check_token(authorization: Optional[str]):
    expected = f"Bearer {ESP_TOKEN}"
    if authorization != expected:
        raise HTTPException(status_code=401, detail="Unauthorized device")


def get_or_create_device(db: Session, device_id: str):
    device = db.query(PlantDevice).filter(PlantDevice.device_id == device_id).first()

    if not device:
        device = PlantDevice(device_id=device_id)
        db.add(device)
        db.commit()
        db.refresh(device)

    return device


def mark_device_seen(db: Session, device: PlantDevice):
    device.is_online = True
    device.last_seen = datetime.utcnow()
    db.commit()


# ================= DEVICE API =================

@app.post("/api/plantation/devices")
def create_device(payload: DeviceCreate):
    db = get_db()

    try:
        existing = db.query(PlantDevice).filter(
            PlantDevice.device_id == payload.device_id
        ).first()

        if existing:
            raise HTTPException(status_code=400, detail="Device already exists")

        device = PlantDevice(**payload.model_dump())
        db.add(device)
        db.commit()
        db.refresh(device)

        return {
            "status": 1,
            "msg": "Device registered successfully",
            "data": {
                "id": device.id,
                "device_id": device.device_id
            }
        }

    finally:
        close_db(db)


@app.get("/api/plantation/devices")
def list_devices():
    db = get_db()

    try:
        devices = db.query(PlantDevice).order_by(PlantDevice.id.desc()).all()

        return {
            "status": 1,
            "data": [
                {
                    "id": d.id,
                    "device_id": d.device_id,
                    "device_name": d.device_name,
                    "location": d.location,
                    "plant_type": d.plant_type,
                    "seed_date": d.seed_date,
                    "is_online": d.is_online,
                    "last_seen": d.last_seen,
                }
                for d in devices
            ]
        }

    finally:
        close_db(db)


# ================= ESP SENSOR LOG API =================

@app.post("/api/plantation/sensor-log")
def create_sensor_log(
    payload: SensorLogCreate,
    authorization: Optional[str] = Header(None)
):
    check_token(authorization)

    db = get_db()

    try:
        device = get_or_create_device(db, payload.device_id)

        data = payload.model_dump()
        data.pop("device_id")

        log = PlantSensorLog(
            device_id_fk=device.id,
            **data
        )

        db.add(log)
        db.commit()

        mark_device_seen(db, device)

        return {
            "status": 1,
            "msg": "Sensor log stored successfully"
        }

    finally:
        close_db(db)


# ================= ESP EVENT LOG API =================

@app.post("/api/plantation/event-log")
def create_event_log(
    payload: EventLogCreate,
    authorization: Optional[str] = Header(None)
):
    check_token(authorization)

    db = get_db()

    try:
        device = get_or_create_device(db, payload.device_id)

        data = payload.model_dump()
        data.pop("device_id")

        log = PlantEventLog(
            device_id_fk=device.id,
            **data
        )

        db.add(log)
        db.commit()

        mark_device_seen(db, device)

        return {
            "status": 1,
            "msg": "Event log stored successfully"
        }

    finally:
        close_db(db)


# ================= FLUTTER GROWTH LOG API =================

@app.post("/api/plantation/growth-log")
def create_growth_log(payload: GrowthLogCreate):
    db = get_db()

    try:
        device = db.query(PlantDevice).filter(
            PlantDevice.device_id == payload.device_id
        ).first()

        if not device:
            raise HTTPException(status_code=404, detail="Device not found")

        data = payload.model_dump()
        data.pop("device_id")

        log = PlantGrowthLog(
            device_id_fk=device.id,
            **data
        )

        db.add(log)
        db.commit()

        return {
            "status": 1,
            "msg": "Growth log stored successfully"
        }

    finally:
        close_db(db)


# ================= READ SENSOR LOGS =================

@app.get("/api/plantation/sensor-logs")
def get_sensor_logs(
    device_id: Optional[str] = None,
    date_from: Optional[date] = Query(None, alias="from"),
    date_to: Optional[date] = Query(None, alias="to"),
    limit: int = 500
):
    db = get_db()

    try:
        query = db.query(PlantSensorLog).join(PlantDevice)

        if device_id:
            query = query.filter(PlantDevice.device_id == device_id)

        if date_from:
            query = query.filter(PlantSensorLog.created_at >= datetime.combine(date_from, datetime.min.time()))

        if date_to:
            query = query.filter(PlantSensorLog.created_at <= datetime.combine(date_to, datetime.max.time()))

        logs = query.order_by(PlantSensorLog.created_at.desc()).limit(limit).all()

        return {
            "status": 1,
            "data": [
                {
                    "device_id": log.device.device_id,
                    "temperature_c": log.temperature_c,
                    "humidity_percent": log.humidity_percent,
                    "soil1_percent": log.soil1_percent,
                    "soil2_percent": log.soil2_percent,
                    "water_percent": log.water_percent,
                    "co2_ppm": log.co2_ppm,
                    "grow_light_power": log.grow_light_power,
                    "wifi_signal": log.wifi_signal,
                    "created_at": log.created_at,
                }
                for log in logs
            ]
        }

    finally:
        close_db(db)


# ================= READ EVENT LOGS =================

@app.get("/api/plantation/event-logs")
def get_event_logs(
    device_id: Optional[str] = None,
    limit: int = 500
):
    db = get_db()

    try:
        query = db.query(PlantEventLog).join(PlantDevice)

        if device_id:
            query = query.filter(PlantDevice.device_id == device_id)

        logs = query.order_by(PlantEventLog.created_at.desc()).limit(limit).all()

        return {
            "status": 1,
            "data": [
                {
                    "device_id": log.device.device_id,
                    "event_type": log.event_type,
                    "event_value": log.event_value,
                    "severity": log.severity,
                    "description": log.description,
                    "created_at": log.created_at,
                }
                for log in logs
            ]
        }

    finally:
        close_db(db)


# ================= READ GROWTH LOGS =================

@app.get("/api/plantation/growth-logs")
def get_growth_logs(device_id: Optional[str] = None):
    db = get_db()

    try:
        query = db.query(PlantGrowthLog).join(PlantDevice)

        if device_id:
            query = query.filter(PlantDevice.device_id == device_id)

        logs = query.order_by(PlantGrowthLog.log_date.desc()).all()

        return {
            "status": 1,
            "data": [
                {
                    "device_id": log.device.device_id,
                    "log_date": log.log_date,
                    "plant_height_cm": log.plant_height_cm,
                    "leaf_count": log.leaf_count,
                    "leaf_color": log.leaf_color,
                    "health_score": log.health_score,
                    "growth_stage": log.growth_stage,
                    "notes": log.notes,
                    "watering_done": log.watering_done,
                    "fertilizer_applied": log.fertilizer_applied,
                    "disease_detected": log.disease_detected,
                    "created_at": log.created_at,
                }
                for log in logs
            ]
        }

    finally:
        close_db(db)


# ================= LATEST STATUS =================

@app.get("/api/plantation/latest-status")
def latest_status(device_id: str):
    db = get_db()

    try:
        latest = db.query(PlantSensorLog).join(PlantDevice).filter(
            PlantDevice.device_id == device_id
        ).order_by(PlantSensorLog.created_at.desc()).first()

        if not latest:
            return {
                "status": 0,
                "msg": "No sensor data found"
            }

        return {
            "status": 1,
            "data": {
                "device_id": latest.device.device_id,
                "temperature_c": latest.temperature_c,
                "humidity_percent": latest.humidity_percent,
                "soil1_percent": latest.soil1_percent,
                "soil2_percent": latest.soil2_percent,
                "water_percent": latest.water_percent,
                "co2_ppm": latest.co2_ppm,
                "grow_light_power": latest.grow_light_power,
                "wifi_signal": latest.wifi_signal,
                "last_updated": latest.created_at,
            }
        }

    finally:
        close_db(db)


# ================= DASHBOARD =================

@app.get("/api/plantation/dashboard")
def dashboard(device_id: str):
    db = get_db()

    try:
        latest = db.query(PlantSensorLog).join(PlantDevice).filter(
            PlantDevice.device_id == device_id
        ).order_by(PlantSensorLog.created_at.desc()).first()

        events = db.query(PlantEventLog).join(PlantDevice).filter(
            PlantDevice.device_id == device_id
        ).order_by(PlantEventLog.created_at.desc()).limit(10).all()

        growth = db.query(PlantGrowthLog).join(PlantDevice).filter(
            PlantDevice.device_id == device_id
        ).order_by(PlantGrowthLog.log_date.desc()).limit(10).all()

        return {
            "status": 1,
            "data": {
                "latest_sensor": {
                    "temperature_c": latest.temperature_c if latest else None,
                    "humidity_percent": latest.humidity_percent if latest else None,
                    "soil1_percent": latest.soil1_percent if latest else None,
                    "soil2_percent": latest.soil2_percent if latest else None,
                    "water_percent": latest.water_percent if latest else None,
                    "co2_ppm": latest.co2_ppm if latest else None,
                    "grow_light_power": latest.grow_light_power if latest else None,
                    "last_updated": latest.created_at if latest else None,
                },
                "recent_events": [
                    {
                        "event_type": e.event_type,
                        "severity": e.severity,
                        "description": e.description,
                        "created_at": e.created_at,
                    }
                    for e in events
                ],
                "growth_progress": [
                    {
                        "log_date": g.log_date,
                        "plant_height_cm": g.plant_height_cm,
                        "leaf_count": g.leaf_count,
                        "growth_stage": g.growth_stage,
                        "health_score": g.health_score,
                    }
                    for g in growth
                ]
            }
        }

    finally:
        close_db(db)

# ================= PLANT PROFILE API =================

@app.post("/api/plantation/plants")
def create_plant_profile(payload: PlantProfileCreate):

    db = get_db()

    try:

        plant = PlantProfile(**payload.model_dump())

        db.add(plant)
        db.commit()

        return {
            "status": 1,
            "msg": "Plant profile created"
        }

    finally:
        close_db(db)

@app.get("/api/plantation/plants")
def get_plant_profiles():

    db = get_db()

    try:
        plants = db.query(PlantProfile).all()

        return {
            "status": 1,
            "data": [
                {
                    "id": p.id,
                    "plant_name": p.plant_name,
                    "soil_moisture_min": p.soil_moisture_min,
                    "soil_moisture_max": p.soil_moisture_max,
                    "temp_min_c": p.temp_min_c,
                    "temp_max_c": p.temp_max_c,
                    "humidity_min": p.humidity_min,
                    "humidity_max": p.humidity_max,
                    "light_hours": p.light_hours,
                    "led_pwm": p.led_pwm,
                    "lumens": p.lumens,
                    "led_height_cm": p.led_height_cm,
                    "growth_days_min": p.growth_days_min,
                    "growth_days_max": p.growth_days_max
                }
                for p in plants
            ]
        }

    finally:
        close_db(db)


# ================= CSV EXPORT =================

@app.get("/api/plantation/export/sensor-logs.csv")
def export_sensor_logs_csv(
    device_id: Optional[str] = None,
    date_from: Optional[date] = Query(None, alias="from"),
    date_to: Optional[date] = Query(None, alias="to")
):
    db = get_db()

    query = db.query(PlantSensorLog).join(PlantDevice)

    if device_id:
        query = query.filter(PlantDevice.device_id == device_id)

    if date_from:
        query = query.filter(PlantSensorLog.created_at >= datetime.combine(date_from, datetime.min.time()))

    if date_to:
        query = query.filter(PlantSensorLog.created_at <= datetime.combine(date_to, datetime.max.time()))

    logs = query.order_by(PlantSensorLog.created_at.asc()).all()

    output = io.StringIO()
    writer = csv.writer(output)

    writer.writerow([
        "device_id",
        "temperature_c",
        "humidity_percent",
        "soil1_raw",
        "soil2_raw",
        "soil1_percent",
        "soil2_percent",
        "water_raw",
        "water_percent",
        "co2_ppm",
        "grow_light_power",
        "grow_light_pwm",
        "wifi_signal",
        "uptime_seconds",
        "seed_day",
        "growth_stage",
        "created_at",
    ])

    for log in logs:
        writer.writerow([
            log.device.device_id,
            log.temperature_c,
            log.humidity_percent,
            log.soil1_raw,
            log.soil2_raw,
            log.soil1_percent,
            log.soil2_percent,
            log.water_raw,
            log.water_percent,
            log.co2_ppm,
            log.grow_light_power,
            log.grow_light_pwm,
            log.wifi_signal,
            log.uptime_seconds,
            log.seed_day,
            log.growth_stage,
            log.created_at,
        ])

    db.close()
    output.seek(0)

    return StreamingResponse(
        output,
        media_type="text/csv",
        headers={
            "Content-Disposition": "attachment; filename=sensor_logs.csv"
        }
    )
