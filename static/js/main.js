/* ========================================================= */
/* === ГЛОБАЛЬНІ ЗМІННІ (Для календаря та відновлення) === */
/* ========================================================= */
let targetDate = null; 
let globalRecords = [];
let currentDateDisplay = new Date();
const monthNames = ["Січень", "Лютий", "Березень", "Квітень", "Травень", "Червень", "Липень", "Серпень", "Вересень", "Жовтень", "Листопад", "Грудень"];
let targetAccount = ""; 

/* ========================================================= */
/* === ІНІЦІАЛІЗАЦІЯ ПРИ ЗАВАНТАЖЕННІ БУДЬ-ЯКОЇ СТОРІНКИ === */
/* ========================================================= */
document.addEventListener("DOMContentLoaded", async () => {
    initTheme();
    initScaleLogic();

    if (document.getElementById('loginForm')) initLogin();
    if (document.getElementById('registerForm')) initRegister();
    if (document.getElementById('resetStep1')) initReset();
    if (document.getElementById('recordForm')) initInputForm(); 
    if (document.getElementById('calendarGrid')) {
        await fetchRecords();
        renderCalendarGrid();
    }
    if (document.getElementById('dynamic-pie-chart')) initStatistics();
    if (document.getElementById('profile-name')) initProfile();
    if (document.getElementById('settings-view')) initSettings();
});

/* ========================================================= */
/* === 1. ЛОГІКА ТЕМИ (Темна / Світла) === */
/* ========================================================= */
function initTheme() {
    const themeToggle = document.getElementById('theme-toggle');
    const currentTheme = localStorage.getItem('theme') || 'light';

    if (currentTheme === 'dark') {
        document.body.setAttribute('data-theme', 'dark');
        if (themeToggle) themeToggle.checked = true;
    }

    if (themeToggle) {
        themeToggle.addEventListener('change', function() {
            if (this.checked) {
                document.body.setAttribute('data-theme', 'dark');
                localStorage.setItem('theme', 'dark');
            } else {
                document.body.removeAttribute('data-theme');
                localStorage.setItem('theme', 'light');
            }
        });
    }
}

/* ========================================================= */
/* === 2. ЛОГІКА АВТОРИЗАЦІЇ / РЕЄСТРАЦІЇ === */
/* ========================================================= */
function initLogin() {
    document.getElementById('loginForm').addEventListener('submit', async function(e) {
        e.preventDefault();
        const data = {
            login: document.getElementById('login').value,
            password: document.getElementById('password').value
        };

        try {
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            });
            const result = await response.json();
            if (result.status === 'success') { window.location.href = '/calendar'; }
            else {
                alert("Невірний логін або пароль!");
            }
        } catch (error) {
            console.error("Помилка:", error);
            alert("Сервер не відповідає.");
        }
    });
}

function initRegister() {
    const form = document.getElementById('registerForm');
    if (!form) return;
    
    form.addEventListener('submit', async function(e) {
        e.preventDefault();
        
        const birthInput = document.getElementById('reg_birth');
        const nameInput = document.getElementById('reg_name');
        const passInput = document.getElementById('reg_pass');

        if (!birthInput.value || !nameInput.value || !passInput.value) {
            alert("Помилка: Заповніть усі поля!");
            return;
        }

        const data = { 
            name: nameInput.value.trim(), 
            birthday: birthInput.value, 
            password: passInput.value 
        };
        
        try {
            const response = await fetch('/api/register', { 
                method: 'POST', 
                headers: { 'Content-Type': 'application/json' }, 
                body: JSON.stringify(data) 
            });
            
            const result = await response.json();
            
            if (response.ok && result.status === 'success') { 
                alert("Реєстрація успішна!"); 
                window.location.href = '/login.html'; 
            } else {
                alert("Помилка: " + (result.message || "Сервер відхилив дані."));
            }
        } catch (error) { 
            alert("Помилка з'єднання з сервером!");
            console.error(error); 
        }
    });
}

/* ========================================================= */
/* === 3. ЛОГІКА ВІДНОВЛЕННЯ ПАРОЛЯ (reset.html) === */
/* ========================================================= */
let resetData = {}; // Зробили змінну глобальною для цього блоку

function initReset() {} 

window.handlePasswordReset = function(e) {
    e.preventDefault();
    resetData = {
        login: document.getElementById('reset_login').value.trim(),
        name: document.getElementById('reset_name').value.trim(),
        birthday: document.getElementById('reset_birth').value.trim()
    };
    
    if (!resetData.login || !resetData.name || !resetData.birthday) { 
        alert("Заповніть усі поля!"); 
        return; 
    }
    document.getElementById('resetStep1').style.display = 'none';
    document.getElementById('resetStep2').style.display = 'block';
};

window.saveNewPassword = async function() {
    const newPass = document.getElementById('new_pass').value.trim();
    if (!newPass) { alert("Введіть новий пароль!"); return; }
    
    resetData.new_password = newPass; 
    
    try {
        const response = await fetch('/api/reset_password_unauth', { 
            method: 'PUT', 
            headers: { 'Content-Type': 'application/json' }, 
            body: JSON.stringify(resetData) 
        });
        const result = await response.json();
        if (result.status === 'success') { 
            alert("Пароль успішно змінено!"); 
            window.location.href = '/login.html'; 
        } else { 
            alert("Дані не збігаються. Акаунт не знайдено."); 
            window.goBackToStep1(); 
        }
    } catch (error) { console.error(error); }
};

window.goBackToStep1 = function() {
    document.getElementById('resetStep2').style.display = 'none';
    document.getElementById('resetStep1').style.display = 'block';
    document.getElementById('new_pass').value = '';
};

/* ========================================================= */
/* === 4. ШКАЛА КВАДРАТІВ ТА СТОРІНКА ВВЕДЕННЯ (input.html) === */
/* ========================================================= */
function initScaleLogic() {
    document.querySelectorAll('.scale-container').forEach(container => {
        const squares = container.querySelectorAll('.square-item');
        const plusBtn = container.querySelector('.plus-btn');
        const minusBtn = container.querySelector('.minus-btn');

        function changeRating(rating) {
            squares.forEach((sq, index) => {
                sq.classList.toggle('filled', index < rating);
            });
            container.setAttribute('data-value', rating);
        }

        if(plusBtn) {
            plusBtn.addEventListener('click', () => {
                let curr = parseInt(container.getAttribute('data-value')) || 0;
                if (curr < squares.length) changeRating(curr + 1);
            });
        }

        if(minusBtn) {
            minusBtn.addEventListener('click', () => {
                let curr = parseInt(container.getAttribute('data-value')) || 0;
                if (curr > 0) changeRating(curr - 1);
            });
        }

        squares.forEach(sq => {
            sq.addEventListener('click', () => {
                changeRating(parseInt(sq.dataset.value));
            });
        });
    });
}

function initInputForm() {
    const form = document.getElementById('recordForm');
    if (!form) return;
    
    form.addEventListener('submit', async function(e) {
        e.preventDefault();
        
        const payload = {
            mood: parseInt(document.getElementById('mood-scale').getAttribute('data-value')) || 0,
            stress: parseInt(document.getElementById('stress-scale').getAttribute('data-value')) || 0,
            energy: parseInt(document.getElementById('energy-scale').getAttribute('data-value')) || 0,
            note: document.getElementById('note-input').value || "",
            hashtag: parseInt(document.getElementById('hashtag-select').value) || 0
        };

        if (payload.mood === 0 || payload.stress === 0 || payload.energy === 0) {
            alert("Будь ласка, оцініть свій стан за всіма шкалами!"); 
            return;
        }

        try {
            const response = await fetch('/api/record', {
                method: 'PUT', 
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
            
            const result = await response.json();
            
            if (response.ok && result.status === 'success') {
                alert("Запис успішно додано!");
                window.location.href = '/calendar'; // Поправив редирект з /home на /calendar
            } else {
                alert("Помилка при збереженні: " + (result.message || ""));
            }
        } catch (error) {
            console.error("Помилка:", error);
            alert("Немає зв'язку з сервером!");
        }
    });
}

/* ========================================================= */
/* === 5. ЛОГІКА КАЛЕНДАРЯ (calendar.html) === */
/* ========================================================= */
async function fetchRecords() {
    try {
        const response = await fetch('/api/records');
        const data = await response.json();
        if (data.status === 'success') globalRecords = data.records;
    } catch (error) {
        console.error("Помилка завантаження даних:", error);
    }
}

window.changeMonth = function(direction) {
    currentDateDisplay.setMonth(currentDateDisplay.getMonth() + direction);
    renderCalendarGrid();
};

function renderCalendarGrid() {
    const grid = document.getElementById('calendarGrid');
    if (!grid) return;

    const year = currentDateDisplay.getFullYear();
    const month = currentDateDisplay.getMonth(); 
    
    document.getElementById('month-year-display').innerText = `${monthNames[month]} ${year}`;
    const daysInMonth = new Date(year, month + 1, 0).getDate();
    grid.innerHTML = ''; 

    const recordsForThisMonth = globalRecords.filter(r => 
        r.entry_date.month === (month + 1) && r.entry_date.year === year
    );

    for (let i = 1; i <= daysInMonth; i++) {
        const cell = document.createElement('div');
        cell.className = 'calendar-day';
        const record = recordsForThisMonth.find(r => r.entry_date.day === i);

        if (record) {
            const averageScore = (record.nastry + record.energy + record.stress) / 3;
            const markerColor = averageScore > 5 ? '#4CAF50' : '#F44336';

            cell.innerHTML = `
                <div class="day-content" style="position: relative; width: 100%; height: 100%; display: flex; justify-content: center; align-items: center;">
                    <span class="day-number" style="position: relative; z-index: 2; font-weight: bold; color: white;">${i}</span>
                    <div class="day-marker" style="position: absolute; width: 80%; height: 80%; border-radius: 50%; z-index: 1; opacity: 0.85; background-color: ${markerColor};"></div>
                </div>
            `;
            cell.onclick = () => openRecordModal(record);
        } else {
            cell.className += ' no-entry';
            cell.innerHTML = `
                <div class="day-content" style="display: flex; flex-direction: column; align-items: center;">
                    <span class="plus-sign" style="font-size: 24px; color: var(--accent-color);">+</span >
                    <span class="day-number">${i}</span>
                </div>
            `;
            cell.onclick = () => openRecordModal(null, i, month + 1, year);
        }
        grid.appendChild(cell);
    }
}

window.setContainerValue = function(id, value) {
    const container = document.getElementById(id);
    if (!container) return;
    container.setAttribute('data-value', value);
    container.querySelectorAll('.square-item').forEach((sq, idx) => {
        sq.classList.toggle('filled', idx < value);
    });
};

window.openRecordModal = function(record, day, month, year) {
    const deleteBtn = document.getElementById('deleteBtn');
    if (record) {
        targetDate = record.entry_date;
        document.getElementById('modal-title').innerText = `Редагувати запис за ${targetDate.day}.${targetDate.month}.${targetDate.year}`;
        setContainerValue('mood-scale', record.nastry);
        setContainerValue('energy-scale', record.energy);
        setContainerValue('stress-scale', record.stress);
        document.getElementById('hashtag-select').value = record.hashtag;
        document.getElementById('note-input').value = record.note;
        deleteBtn.style.display = 'block'; 
    } else {
        targetDate = { day, month, year };
        document.getElementById('modal-title').innerText = `Новий запис за ${day}.${month}.${year}`;
        setContainerValue('mood-scale', 0);
        setContainerValue('energy-scale', 0);
        setContainerValue('stress-scale', 0);
        document.getElementById('hashtag-select').value = '0';
        document.getElementById('note-input').value = '';
        deleteBtn.style.display = 'none'; 
    }
    document.getElementById('recordModal').style.display = 'flex';
};

window.closeModal = function() {
    document.getElementById('recordModal').style.display = 'none';
};

window.saveRecordFromCalendar = async function(e) {
    if (e) e.preventDefault(); 
    
    const data = { 
        mood: parseInt(document.getElementById('mood-scale').getAttribute('data-value')) || 0, 
        energy: parseInt(document.getElementById('energy-scale').getAttribute('data-value')) || 0, 
        stress: parseInt(document.getElementById('stress-scale').getAttribute('data-value')) || 0, 
        hashtag: parseInt(document.getElementById('hashtag-select').value) || 0, 
        note: document.getElementById('note-input').value || "", 
        day: targetDate.day, 
        month: targetDate.month, 
        year: targetDate.year 
    };
    
    if (data.mood === 0 || data.energy === 0 || data.stress === 0) { 
        alert("Заповніть усі шкали!"); 
        return; 
    }
    
    try {
        const response = await fetch('/api/record', { 
            method: 'PUT', 
            headers: { 'Content-Type': 'application/json' }, 
            body: JSON.stringify(data) 
        });
        const result = await response.json();
        
        if (response.ok && result.status === 'success') { 
            closeModal(); 
            await fetchRecords(); 
            renderCalendarGrid(); 
        } else { 
            alert("Помилка збереження: " + (result.message || "")); 
        }
    } catch (error) { console.error(error); }
};

window.deleteCurrentRecord = async function(e) {
    if (e) e.preventDefault(); 
    
    if (!targetDate) {
        alert("Немає запису для видалення!");
        return;
    }

    if (!confirm("Видалити цей запис?")) return;
    
    try {
        const response = await fetch('/api/delete', { 
            method: 'DELETE', 
            headers: { 'Content-Type': 'application/json' }, 
            body: JSON.stringify({
                day: targetDate.day,
                month: targetDate.month,
                year: targetDate.year
            }) 
        });
        
        const result = await response.json();
        
        if (response.ok && result.status === 'success') { 
            closeModal(); 
            await fetchRecords(); 
            renderCalendarGrid(); 
            alert("Запис успішно видалено!");
        } else {
            alert("Помилка видалення: " + (result.message || "Сервер не зміг видалити файл."));
        }
    } catch (err) { 
        console.error(err); 
        alert("Помилка з'єднання з сервером при видаленні!");
    }
};

/* ========================================================= */
/* === 6. ЛОГІКА СТАТИСТИКИ (statistics.html) === */
/* ========================================================= */
function initStatistics() {
    const startDateInput = document.getElementById('start-date');
    const endDateInput = document.getElementById('end-date');
    const hashtagSelect = document.getElementById('hashtag-select');
    
    if (!startDateInput || !endDateInput || !hashtagSelect) return;

    const savedStart = localStorage.getItem('saved_start_date');
    const savedEnd = localStorage.getItem('saved_end_date');
    const savedHashtag = localStorage.getItem('saved_hashtag');

    if (savedStart && savedEnd) {
        startDateInput.value = savedStart;
        endDateInput.value = savedEnd;
    } else {
        const today = new Date();
        const lastMonth = new Date();
        lastMonth.setMonth(today.getMonth() - 1);
        
        endDateInput.value = today.toISOString().split('T')[0];
        startDateInput.value = lastMonth.toISOString().split('T')[0];
    }

    if (savedHashtag) {
        hashtagSelect.value = savedHashtag;
    }

    startDateInput.addEventListener('change', updateStatistics);
    endDateInput.addEventListener('change', updateStatistics);
    hashtagSelect.addEventListener('change', updateStatistics);
    
    updateStatistics();
}

window.updateStatistics = async function() {
    const start = document.getElementById('start-date').value;
    const end = document.getElementById('end-date').value;
    const hashtag = parseInt(document.getElementById('hashtag-select').value);

    if (!start || !end) return;

    try {
        const response = await fetch('/api/stats_data', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ start, end, hashtag })
        });
        const data = await response.json();

        if (data.status === 'success') {
            const chart = document.getElementById('dynamic-pie-chart');
            const info = document.getElementById('stats-info');

            if (data.count === 0 || (data.avg_nastry === 0 && data.avg_stress === 0 && data.avg_energy === 0)) {
                chart.style.background = '#e0e0e0';
                info.innerText = "Немає записів за цей період";
                return;
            }

            const total = data.avg_nastry + data.avg_stress + data.avg_energy;
            const p1 = (data.avg_nastry / total) * 100;
            const p2 = p1 + (data.avg_stress / total) * 100;

            chart.style.background = `conic-gradient(
                var(--bg-body) 0% ${p1}%, 
                var(--accent-color) ${p1}% ${p2}%, 
                #1e2a80 ${p2}% 100%
            )`;
            info.innerText = `Знайдено записів: ${data.count}`;
        }
    } catch (error) { console.error("Помилка:", error); }
};

/* ========================================================= */
/* === 7. ЛОГІКА ПРОФІЛЮ (profile.html) === */
/* ========================================================= */
async function initProfile() {
    try {
        const response = await fetch('/api/profile_data');
        const data = await response.json();
        
        if (data.status === 'success') {
            // Прибрано вивід прізвища (surname), щоб не було "undefined"
            document.getElementById('profile-name').innerText = data.name;
            document.getElementById('profile-birthday').innerText = `день народження: ${data.birthday}`;
            document.getElementById('profile-reg-date').innerText = `ID користувача: ${data.id}`;
            document.getElementById('profile-days-together').innerText = `всього записів: ${data.total_records}`;
            
            document.getElementById('bar-mood').style.height = `${(data.avg_mood / 10) * 150}px`;
            document.getElementById('bar-stress').style.height = `${(data.avg_stress / 10) * 150}px`;
            document.getElementById('bar-energy').style.height = `${(data.avg_energy / 10) * 150}px`;
        }
    } catch (error) {
        document.getElementById('profile-name').innerText = "Помилка з'єднання";
    }

    const logoutBtn = document.getElementById('logout-btn');
    if (logoutBtn) {
        logoutBtn.addEventListener('click', async () => {
            if (confirm("Ви впевнені, що хочете вийти з акаунта?")) {
                try {
                    const response = await fetch('/api/logout', { method: 'POST' });
                    if (response.ok) {
                        window.location.href = '/login.html'; 
                    }
                } catch (error) {
                    console.error("Помилка при виході:", error);
                }
            }
        });
    }
}

/* ========================================================= */
/* === 8. ЛОГІКА НАЛАШТУВАНЬ (settings.html) === */
/* ========================================================= */
function initSettings() {
    const savePassBtn = document.getElementById('save-password-btn') || document.getElementById('change-password-btn') || document.querySelector('.save-password-btn');
    const passInput = document.getElementById('new-password-input') || document.querySelector('.new-password-input');
    
    if (savePassBtn && passInput) {
        savePassBtn.addEventListener('click', async () => {
            const newPassword = passInput.value.trim();
            if (!newPassword) { 
                alert('Будь ласка, введіть новий пароль!'); 
                return; 
            }
            
            try {
                const response = await fetch('/api/change_password', {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: newPassword }) 
                });
                
                if (response.status === 401) {
                    alert("Сесія закінчилася або ви не авторизовані. Будь ласка, увійдіть в акаунт.");
                    window.location.href = '/login.html';
                    return;
                }

                const result = await response.json();
                if (result.status === 'success') {
                    alert('Пароль успішно змінено!');
                    passInput.value = ''; 
                } else {
                    alert('Помилка: ' + (result.message || 'Не вдалося змінити пароль.'));
                }
            } catch (error) { 
                console.error(error);
                alert('Немає зв\'язку з сервером.'); 
            }
        });
    }

    const saveFiltersBtn = document.getElementById('save-filters-btn');
    if (saveFiltersBtn) {
        saveFiltersBtn.addEventListener('click', () => {
            const startDate = document.getElementById('filter-start-date').value;
            const endDate = document.getElementById('filter-end-date').value;
            const hashtag = document.getElementById('filter-hashtag').value;
            
            if(!startDate || !endDate) { 
                alert('Оберіть обидві дати!'); 
                return; 
            }

            localStorage.setItem('saved_start_date', startDate);
            localStorage.setItem('saved_end_date', endDate);
            localStorage.setItem('saved_hashtag', hashtag);
            alert('Налаштування фільтрів збережено!');
        });
    }

    const deleteAccountBtn = document.getElementById('delete-account-btn');
    const deletePassInput = document.getElementById('delete-password-input');

    if (deleteAccountBtn && deletePassInput) {
        deleteAccountBtn.addEventListener('click', async () => {
            const password = deletePassInput.value.trim();
            if (!password) {
                alert('Будь ласка, введіть пароль для підтвердження!');
                return;
            }
            
            if (!confirm('Ви впевнені, що хочете назавжди видалити свій профіль?')) return;
            
            try {
                const response = await fetch('/api/delete_account', {
                    method: 'DELETE', 
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: password })
                });
                
                if (response.status === 401) {
                    alert("Помилка авторизації.");
                    return;
                }

                const result = await response.json();
                
                if (result.status === 'success') {
                    alert('Акаунт успішно видалено.');
                    window.location.href = '/login.html'; 
                } else {
                    alert('Невірний пароль! Операцію скасовано.');
                    deletePassInput.value = '';
                }
            } catch (error) {
                console.error(error);
                alert('Помилка з\'єднання з сервером.');
            }
        });
    }
}