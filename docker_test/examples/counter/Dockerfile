FROM python:3.12-slim-bookworm
WORKDIR /app
COPY app.py .
ENV PYTHONUNBUFFERED=1 DATA_DIR=/data PORT=8080
EXPOSE 8080
CMD ["python", "app.py"]
